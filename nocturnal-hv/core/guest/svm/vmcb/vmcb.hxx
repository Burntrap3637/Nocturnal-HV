#pragma once

extern "C" void RtlCaptureContext( CONTEXT* ctx );

namespace guest {
    _IRQL_requires_max_( DISPATCH_LEVEL )
        _IRQL_requires_min_( PASSIVE_LEVEL )
        _IRQL_requires_same_
        extern "C" bool launch_vm( void* );

    extern "C" void _sldt( __out segment_selector_t* );
    extern "C" void _str( __out segment_selector_t* );

    namespace vmcb {
        bool is_hypervisor_installed( ) {
            int regs[ 4 ]{};
            __cpuid( regs, 0x40000000 );

            char vendor[ 13 ]{};
            std::memcpy( vendor + 0, &regs[ 1 ], 4 );
            std::memcpy( vendor + 4, &regs[ 2 ], 4 );
            std::memcpy( vendor + 8, &regs[ 3 ], 4 );

            return std::strcmp( vendor, "Nocturnal   " );
        }

        void setup_control_area( virtual_processor_data_t* vp_data ) {
            auto& ctrl = vp_data->m_guest_vmcb.m_control_area;

            ctrl.m_intercept_misc1 |= svm_intercept_misc1_cpuid
                | svm_intercept_misc1_msr_prot
                | svm_intercept_misc1_xsetbv;

            ctrl.m_intercept_misc2 |= svm_intercept_misc2_vmrun;

            if ( m_shared_state->m_nrip_save )
                ctrl.m_intercept_misc2 |= svm_intercept_misc2_nrip_save;

            ctrl.m_msrpm_base_pa =
                kernel::mm_get_physical_address( m_shared_state->m_msrpm ).m_quad_part;
            ctrl.m_guest_asid = 1;
            ctrl.m_np_enable |= svm_np_enable_np_enable;
            ctrl.m_n_cr3 = npt::get_pml4_pa( );
        }

        void setup_state_save_area( virtual_processor_data_t* vp_data, CONTEXT* ctx ) {
            gdtr_t gdtr{};
            idtr_t idtr{};
            _sgdt( &gdtr );
            __sidt( &idtr );

            auto& state = vp_data->m_guest_vmcb.m_state_save_area;

            state.m_cs_selector = ctx->SegCs;
            state.m_cs_attrib = utility::get_access_right( ctx->SegCs, gdtr.m_base );
            state.m_cs_limit = __segmentlimit( ctx->SegCs );
            state.m_cs_base = 0;

            state.m_ss_selector = ctx->SegSs;
            state.m_ss_attrib = utility::get_access_right( ctx->SegSs, gdtr.m_base );
            state.m_ss_limit = __segmentlimit( ctx->SegSs );
            state.m_ss_base = 0;

            state.m_ds_selector = ctx->SegDs;
            state.m_ds_attrib = utility::get_access_right( ctx->SegDs, gdtr.m_base );
            state.m_ds_limit = __segmentlimit( ctx->SegDs );
            state.m_ds_base = 0;

            state.m_es_selector = ctx->SegEs;
            state.m_es_attrib = utility::get_access_right( ctx->SegEs, gdtr.m_base );
            state.m_es_limit = __segmentlimit( ctx->SegEs );
            state.m_es_base = 0;

            std::uint16_t tr_sel = 0, ldtr_sel = 0;
            _str( reinterpret_cast< segment_selector_t* >( &tr_sel ) );
            _sldt( reinterpret_cast< segment_selector_t* >( &ldtr_sel ) );
            state.m_tr_selector = tr_sel;
            state.m_ldtr_selector = ldtr_sel;

            state.m_gdtr_base = gdtr.m_base;
            state.m_gdtr_limit = gdtr.m_limit;
            state.m_idtr_base = idtr.m_base;
            state.m_idtr_limit = idtr.m_limit;

            state.m_cr0 = __readcr0( );
            state.m_cr2 = __readcr2( );
            state.m_cr3 = __readcr3( );
            state.m_cr4 = __readcr4( );
            state.m_efer = __readmsr( ia32_msr_efer );
            state.m_g_pat = __readmsr( ia32_msr_pat );

            state.m_rflags = ctx->EFlags;
            state.m_rsp = ctx->Rsp;
            state.m_rip = ctx->Rip;

            kernel::dbg_print( "[vmcb] guest entry rip=0x%llx rsp=0x%llx rflags=0x%llx\n",
                state.m_rip, state.m_rsp, state.m_rflags );
        }

        void setup_vmcb( virtual_processor_data_t* vp_data, CONTEXT* ctx ) {
            const auto guest_vmcb_pa =
                kernel::mm_get_physical_address( &vp_data->m_guest_vmcb ).m_quad_part;
            const auto host_vmcb_pa =
                kernel::mm_get_physical_address( &vp_data->m_host_vmcb ).m_quad_part;
            const auto host_state_pa =
                kernel::mm_get_physical_address( &vp_data->m_host_state_area ).m_quad_part;

            setup_control_area( vp_data );
            setup_state_save_area( vp_data, ctx );

            __svm_vmsave( guest_vmcb_pa );

            __writemsr( svm_msr_vm_hsave_pa, host_state_pa );
            __svm_vmsave( host_vmcb_pa );

            vp_data->m_host_stack_layout.guest_vmcb_pa = guest_vmcb_pa;
            vp_data->m_host_stack_layout.host_vmcb_pa = host_vmcb_pa;
            vp_data->m_host_stack_layout.self = vp_data;
            vp_data->m_host_stack_layout.shared_vp_data = m_shared_state;
            vp_data->m_host_stack_layout.reserved1 = MAXUINT64;

            kernel::dbg_print( "[vmcb] guest_vmcb_pa=0x%llx host_vmcb_pa=0x%llx\n",
                guest_vmcb_pa, host_vmcb_pa );
        }

        virtual_processor_data_t* alloc_vp_data( std::uint32_t idx ) {
            auto* data = reinterpret_cast< virtual_processor_data_t* >(
                ExAllocatePool2( POOL_FLAG_NON_PAGED, sizeof( virtual_processor_data_t ), 'vpDT' ) );

            if ( !data )
                return nullptr;

            RtlZeroMemory( data, sizeof( virtual_processor_data_t ) );
            data->m_host_stack_layout.reserved1 = MAXUINT64;
            return data;
        }

        bool virtualize_processor( std::uint32_t idx ) {
            auto* ctx = reinterpret_cast< CONTEXT* >(
                ExAllocatePool2( POOL_FLAG_NON_PAGED, sizeof( CONTEXT ), 'txCN' ) );
            if ( !ctx )
                return false;

            auto* data = reinterpret_cast< virtual_processor_data_t* >(
                memory::alloc_contiguous<void*>( sizeof( virtual_processor_data_t ) ) );
            if ( !data ) {
                ExFreePoolWithTag( ctx, 'txCN' );
                return false;
            }

            RtlZeroMemory( data, sizeof( virtual_processor_data_t ) );
            data->m_host_stack_layout.reserved1 = MAXUINT64;

            RtlCaptureContext( ctx );

            if ( m_processor_states[ idx ] != nullptr &&
                m_processor_states[ idx ]->m_host_stack_layout.guest_vmcb_pa != 0 ) {
                kernel::dbg_print( "[nocturnal] processor %u re-entered\n", idx );
                ExFreePoolWithTag( ctx, 'txCN' );
                return true;
            }

            m_processor_states[ idx ] = data;

            kernel::dbg_print( "[nocturnal] virtualizing processor %u\n", idx );

            __writemsr( ia32_msr_efer, __readmsr( ia32_msr_efer ) | efer_svme );
            setup_vmcb( data, ctx );
            ExFreePoolWithTag( ctx, 'txCN' );

            launch_vm( &data->m_host_stack_layout.guest_vmcb_pa );
            KeBugCheck( MANUALLY_INITIATED_CRASH );
        }

        bool devirtualize_processor( std::uint32_t idx ) {
            int regs[ 4 ]{};
            __cpuidex( regs, cpuid_unload, cpuid_unload );

            if ( regs[ 2 ] != 'SSVM' ) {
                kernel::dbg_print( "[nocturnal] processor %u not virtualized\n", idx );
                return false;
            }

            auto* vp_data = reinterpret_cast< virtual_processor_data_t* >(
                ( static_cast< std::uint64_t >( regs[ 3 ] ) << 32 )
                | static_cast< std::uint32_t >( regs[ 0 ] ) );

            //if ( vp_data->m_guest_vmcb )
            //    memory::free_contiguous( vp_data->m_guest_vmcb );
            //if ( vp_data->m_host_vmcb )
            //    memory::free_contiguous( vp_data->m_host_vmcb );
            //if ( vp_data->m_host_state_area )
            //    memory::free_contiguous( vp_data->m_host_state_area );

            ExFreePoolWithTag( vp_data, 'vpDT' );
            m_processor_states[ idx ] = nullptr;

            kernel::dbg_print( "[nocturnal] processor %u devirtualized\n", idx );
            return true;
        }
    }
}