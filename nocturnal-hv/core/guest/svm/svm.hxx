#pragma once
#include <stdarg.h>

namespace guest {
    struct virtualize_dpc_context_t {
        std::uint32_t   m_processor_idx;
        KEVENT          m_completion;
        bool            m_success;
    };

    bool is_svm_supported( ) {
        int regs[ 4 ]{};
        __cpuid( regs, cpuid_max_standard_fn );
        if ( regs[ 1 ] != 'htuA' || regs[ 3 ] != 'itne' || regs[ 2 ] != 'DMAc' ) {
            kernel::dbg_print( "[nocturnal] processor is not amd\n" );
            return false;
        }
        __cpuid( regs, cpuid_processor_features_ex );
        if ( !( regs[ 2 ] & cpuid_ecx_svm ) ) {
            kernel::dbg_print( "[nocturnal] svm is not supported on this processor\n" );
            return false;
        }
        __cpuid( regs, cpuid_svm_features );
        if ( !( regs[ 3 ] & cpuid_edx_np ) ) {
            kernel::dbg_print( "[nocturnal] nested paging is not supported on this processor\n" );
            return false;
        }

        if ( !( regs[ 3 ] & cpuid_edx_nrip_save ) ) {
            kernel::dbg_print( "[nocturnal] nrip save is not supported — cannot continue\n" );
            return false;
        }

        if ( __readmsr( svm_msr_vm_cr ) & svm_vm_cr_svmdis ) {
            kernel::dbg_print( "[nocturnal] svm is disabled by vm_cr.svmdis (bios setting)\n" );
            return false;
        }

        return true;
    }

    static void virtualize_dpc_routine(
        KDPC* dpc,
        void* deferred_context,
        void*,
        void* ) {
        UNREFERENCED_PARAMETER( dpc );

        auto* ctx = reinterpret_cast< virtualize_dpc_context_t* >( deferred_context );
        ctx->m_success = guest::vmcb::virtualize_processor( ctx->m_processor_idx );
        kernel::ke_set_event( &ctx->m_completion, 0, false );
    }

    bool virtualize_all_processors( ) {
        const auto processor_count =
            static_cast< std::uint32_t >( kernel::ke_query_active_processor_count( ) );

        auto* dpcs = reinterpret_cast< KDPC* >(
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof( KDPC ) * processor_count,
                'cpdV' ) );

        auto* contexts = reinterpret_cast< virtualize_dpc_context_t* >(
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof( virtualize_dpc_context_t ) * processor_count,
                'xcpV' ) );

        if ( !dpcs || !contexts ) {
            kernel::dbg_print( "[nocturnal] failed to allocate DPC arrays\n" );
            if ( dpcs )      ExFreePoolWithTag( dpcs, 'cpdV' );
            if ( contexts )  ExFreePoolWithTag( contexts, 'xcpV' );
            return false;
        }

        RtlZeroMemory( dpcs, sizeof( KDPC ) * processor_count );
        RtlZeroMemory( contexts, sizeof( virtualize_dpc_context_t ) * processor_count );

        for ( std::uint32_t idx = 0; idx < processor_count; idx++ ) {
            auto& ctx = contexts[ idx ];
            ctx.m_processor_idx = idx;
            ctx.m_success = false;
            kernel::ke_initialize_event( &ctx.m_completion, SynchronizationEvent, false );

            processor_number_t proc_number{};
            if ( !kernel::ke_get_processor_number_from_index( idx, &proc_number ) ) {
                kernel::dbg_print( "[nocturnal] ke_get_processor_number_from_index failed for %u\n", idx );
                ExFreePoolWithTag( dpcs, 'cpdV' );
                ExFreePoolWithTag( contexts, 'xcpV' );
                return false;
            }

            kernel::ke_initialize_dpc( &dpcs[ idx ], virtualize_dpc_routine, &ctx );
            kernel::ke_set_importance_dpc( &dpcs[ idx ], HighImportance );

            if ( NT_SUCCESS( kernel::ke_set_target_processor_dpc_ex(
                &dpcs[ idx ],
                reinterpret_cast< PPROCESSOR_NUMBER >( &proc_number ) ) ) ) {
                kernel::ke_insert_queue_dpc( &dpcs[ idx ], nullptr, nullptr );
            }
            else {
                kernel::dbg_print( "[nocturnal] ke_set_target_processor_dpc_ex failed for %u\n", idx );
                ExFreePoolWithTag( dpcs, 'cpdV' );
                ExFreePoolWithTag( contexts, 'xcpV' );
                return false;
            }
        }

        bool all_success = true;
        for ( std::uint32_t idx = 0; idx < processor_count; idx++ ) {
            kernel::ke_wait_for_single_object(
                &contexts[ idx ].m_completion,
                Executive,
                KernelMode,
                false,
                nullptr );

            if ( !contexts[ idx ].m_success ) {
                kernel::dbg_print( "[nocturnal] processor %u failed to virtualize\n", idx );
                all_success = false;
            }
        }

        ExFreePoolWithTag( dpcs, 'cpdV' );
        ExFreePoolWithTag( contexts, 'xcpV' );

        if ( all_success ) {
            kernel::dbg_print( "[nocturnal] all %u processors virtualized\n", processor_count );
        }

        return all_success;
    }

    bool initialize( ) {
        if ( !is_svm_supported( ) )
            return false;

        m_shared_state = memory::alloc_page< shared_state_t* >( sizeof( shared_state_t ) );
        if ( !m_shared_state ) {
            kernel::dbg_print( "[nocturnal] could not allocate state\n" );
            return false;
        }

        int regs[ 4 ]{};
        __cpuid( regs, 0x8000000A );  // SVM feature flags in EDX
        m_shared_state->m_nrip_save = ( regs[ 3 ] >> 3 ) & 1;  // bit 3 of EDX
        m_shared_state->m_decode_assists = ( regs[ 3 ] >> 7 ) & 1; // bit 7 of EDX
        kernel::dbg_print( "[nocturnal] nrip_save=%u decode_assists=%u\n",
            m_shared_state->m_nrip_save,
            m_shared_state->m_decode_assists );

        const auto processor_count = kernel::ke_query_active_processor_count( );
        m_processor_states = memory::alloc_page< virtual_processor_data_t** >( sizeof( shared_state_t ) * processor_count );
        if ( !m_processor_states ) {
            kernel::dbg_print( "[nocturnal] could not allocate states\n" );
            return false;
        }

        m_shared_state->m_msrpm = memory::alloc_contiguous< void* >( svm_msr_permissions_map_size );
        if ( !m_shared_state->m_msrpm ) {
            kernel::dbg_print( "[nocturnal] could not allocate msrpm\n" );
            return false;
        }

        if ( !npt::build_identity_map( ) ) {
            kernel::dbg_print( "[nocturnal] failed to build identity map\n" );
            return false;
        }
        kernel::dbg_print( "[nocturnal] identity map built\n" );

        npt::build_msrpm( );
        kernel::dbg_print( "[nocturnal] msrpm built\n" );

        virtualize_all_processors( );
        kernel::dbg_print( "[nocturnal] virtualize_all_processors finished\n" );

        //if ( guest::vmcb::is_hypervisor_installed( ) ) {
        //    kernel::dbg_print( "[nocturnal] hypervisor installed successfully\n" );
        //    return true;
        //}

        //kernel::dbg_print( "[nocturnal] failed to verify hypervisor installation\n" );
        return false;
    }

    void shutdown( ) {
        utility::execute_on_each_processor( [ & ]( std::uint32_t idx ) -> bool {
            vmcb::devirtualize_processor( idx );
            return true;
            } );

        if ( m_shared_state ) {
            //if ( m_shared_state->m_msrpm )
            //    kernel::mm_free_contiguous_memory( m_shared_state->m_msrpm );
            memory::free_page( m_shared_state, sizeof( shared_state_t ) );
            m_shared_state = nullptr;
        }

        kernel::dbg_print( "[nocturnal] shutdown complete\n" );
    }
}