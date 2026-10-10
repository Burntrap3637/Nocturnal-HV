#pragma once

namespace kernel {
    void* mm_get_virtual_for_physical( std::uintptr_t phys_addr ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "MmGetVirtualForPhysical" ) );
            if ( !export_address ) return nullptr;
        }

        using function_t = void* ( * )( std::uintptr_t physical_address );
        return reinterpret_cast< function_t >( export_address )( phys_addr );
    }

    physical_address_t mm_get_physical_address( void* virtual_address ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "MmGetPhysicalAddress" ) );
            if ( !export_address ) return { };
        }

        using function_t = physical_address_t( * )( void* virtual_address );
        return reinterpret_cast< function_t >( export_address )( virtual_address );
    }

    physical_memory_range_t* mm_get_physical_memory_ranges( ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "MmGetPhysicalMemoryRanges" ) );
            if ( !export_address ) return nullptr;
        }

        using function_t = physical_memory_range_t * ( void );
        return reinterpret_cast< function_t* >( export_address )( );
    }

    void* mm_allocate_independent_pages( std::size_t size ) {
        static auto mm_allocate_independent_pages = 0ull;
        if ( !mm_allocate_independent_pages ) {
            mm_allocate_independent_pages = m_pdb.m_mm_allocate_independent_pages;
            if ( !mm_allocate_independent_pages )
                return nullptr;
        }

        using function_t = void* ( std::uint64_t, int, std::uint64_t, unsigned int );
        return reinterpret_cast< function_t* >( mm_allocate_independent_pages )( size, -1, 0, 0 );
    }

    void mm_free_contiguous_memory( void* base_address ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "MmFreeContiguousMemory" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( void* );
        reinterpret_cast< function_t >( export_address )( base_address );
    }

    void mm_free_independent_pages( std::uint64_t independent_pages, std::size_t size ) {
        static auto mm_free_independent_pages = 0ull;
        if ( !mm_free_independent_pages ) {
            mm_free_independent_pages = m_pdb.m_mm_free_independent_pages;
            if ( !mm_free_independent_pages )
                return;
        }

        using function_t = void( std::uint64_t, int );
        reinterpret_cast< function_t* >( mm_free_independent_pages )( independent_pages, size );
    }

    void* mm_allocate_contiguous_node_memory( std::size_t size ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "MmAllocateContiguousNodeMemory" ) );
            if ( !export_address ) return nullptr;
        }

        physical_address_t lowest{ };
        physical_address_t highest{ };
        physical_address_t boundary{ };

        lowest.m_quad_part = 0;
        highest.m_quad_part = -1;
        boundary.m_quad_part = 0;

        using function_t = void* ( * )(
            std::size_t,
            physical_address_t,
            physical_address_t,
            physical_address_t,
            std::uint32_t,
            std::uint32_t
            );

        auto result = reinterpret_cast< function_t >( export_address )(
            size,
            lowest,
            highest,
            boundary,
            0x04,         // Protect = PAGE_READWRITE
            0xFFFFFFFF    // PreferredNode = MM_ANY_NODE_OK
            );

        if ( result )
            memset( result, 0, size );

        return result;
    }

    std::uint32_t ke_query_active_processor_count( ) {
        static auto ke_query_active_processor_count_ex = 0ull;
        if ( !ke_query_active_processor_count_ex ) {
            ke_query_active_processor_count_ex = m_pdb.m_ke_query_active_processor_count_ex;
            if ( !ke_query_active_processor_count_ex ) return {};
        }

        using function_t = std::uint32_t( std::uint32_t );
        return reinterpret_cast< function_t* >( ke_query_active_processor_count_ex )( 0xFFFF );
    }

    bool ke_get_processor_number_from_index( std::uint32_t index, processor_number_t* processor_number ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeGetProcessorNumberFromIndex" ) );
            if ( !export_address ) return false;
        }

        using function_t = std::int32_t( * )( std::uint32_t, processor_number_t* );
        return reinterpret_cast< function_t >( export_address )( index, processor_number ) >= 0;
    }

    void ke_set_system_group_affinity_thread( group_affinity_t* affinity, group_affinity_t* old_affinity ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeSetSystemGroupAffinityThread" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( group_affinity_t*, group_affinity_t* );
        reinterpret_cast< function_t >( export_address )( affinity, old_affinity );
    }

    void ke_revert_to_user_group_affinity_thread( group_affinity_t* old_affinity ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeRevertToUserGroupAffinityThread" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( group_affinity_t* );
        reinterpret_cast< function_t >( export_address )( old_affinity );
    }

    void rtl_capture_context( CONTEXT* ctx ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "RtlCaptureContext" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( CONTEXT* );
        reinterpret_cast< function_t >( export_address )( ctx );
    }

    template<class... args_t>
    std::int8_t dbg_print( const char* format, args_t... va_args ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "DbgPrintEx" ) );
            if ( !export_address ) return 0;
        }

        using function_t = std::int32_t( std::uint32_t flag, std::uint32_t level,
            const char* format, args_t... va_args );
        return reinterpret_cast< function_t* >( export_address )( 0, 0, format, va_args... ) ==
            nt_status_t::success;
    }

    void* ex_create_callback(
        void** callback_object,
        object_attributes_t* object_attributes,
        bool create,
        bool allow_multiple_callbacks ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "ExCreateCallback" ) );
            if ( !export_address ) return nullptr;
        }

        using function_t = std::int32_t( * )(
            void**,
            object_attributes_t*,
            bool,
            bool );

        auto status = reinterpret_cast< function_t >( export_address )(
            callback_object,
            object_attributes,
            create,
            allow_multiple_callbacks );

        return status >= 0 ? *callback_object : nullptr;
    }

    void* ex_register_callback(
        void* callback_object,
        void* callback_function,
        void* callback_context ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "ExRegisterCallback" ) );
            if ( !export_address ) return nullptr;
        }

        using function_t = void* ( * )(
            void*,
            void*,
            void* );

        return reinterpret_cast< function_t >( export_address )(
            callback_object,
            callback_function,
            callback_context );
    }

    void ex_unregister_callback( void* callback_registration ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "ExUnregisterCallback" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( void* );
        reinterpret_cast< function_t >( export_address )( callback_registration );
    }

    void obf_dereference_object( void* object ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            // ObDereferenceObject is a macro over ObfDereferenceObject on x64
            export_address = get_export( m_ntoskrnl_base, oxorany( "ObfDereferenceObject" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( void* );
        reinterpret_cast< function_t >( export_address )( object );
    }

    void ke_lower_irql( std::uint8_t new_irql ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeLowerIrql" ) );
            if ( !export_address ) return;
        }

        using function_t = void( * )( std::uint8_t );
        reinterpret_cast< function_t >( export_address )( new_irql );
    }

    std::uint8_t ke_raise_irql_to_dpc_level( ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeRaiseIrqlToDpcLevel" ) );
            if ( !export_address ) return 0;
        }

        using function_t = std::uint8_t( * )( );
        return reinterpret_cast< function_t >( export_address )( );
    }


    // KeInitializeDpc
    void ke_initialize_dpc( KDPC* dpc, PKDEFERRED_ROUTINE routine, void* context ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeInitializeDpc" ) );
            if ( !export_address ) return;
        }
        using function_t = void( * )( KDPC*, PKDEFERRED_ROUTINE, void* );
        return reinterpret_cast< function_t >( export_address )( dpc, routine, context );
    }

    // KeSetTargetProcessorDpcEx
    NTSTATUS ke_set_target_processor_dpc_ex( KDPC* dpc, PPROCESSOR_NUMBER proc_number ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeSetTargetProcessorDpcEx" ) );
            if ( !export_address ) return STATUS_NOT_FOUND;
        }
        using function_t = NTSTATUS( * )( KDPC*, PPROCESSOR_NUMBER );
        return reinterpret_cast< function_t >( export_address )( dpc, proc_number );
    }

    // KeSetImportanceDpc
    void ke_set_importance_dpc( KDPC* dpc, KDPC_IMPORTANCE importance ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeSetImportanceDpc" ) );
            if ( !export_address ) return;
        }
        using function_t = void( * )( KDPC*, KDPC_IMPORTANCE );
        return reinterpret_cast< function_t >( export_address )( dpc, importance );
    }

    // KeInsertQueueDpc
    bool ke_insert_queue_dpc( KDPC* dpc, void* system_arg1, void* system_arg2 ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeInsertQueueDpc" ) );
            if ( !export_address ) return false;
        }
        using function_t = bool( * )( KDPC*, void*, void* );
        return reinterpret_cast< function_t >( export_address )( dpc, system_arg1, system_arg2 );
    }

    // KeInitializeEvent
    void ke_initialize_event( KEVENT* event, EVENT_TYPE type, bool state ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeInitializeEvent" ) );
            if ( !export_address ) return;
        }
        using function_t = void( * )( KEVENT*, EVENT_TYPE, BOOLEAN );
        return reinterpret_cast< function_t >( export_address )( event, type, state ? TRUE : FALSE );
    }

    // KeSetEvent
    LONG ke_set_event( KEVENT* event, KPRIORITY increment, bool wait ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeSetEvent" ) );
            if ( !export_address ) return 0;
        }
        using function_t = LONG( * )( KEVENT*, KPRIORITY, BOOLEAN );
        return reinterpret_cast< function_t >( export_address )( event, increment, wait ? TRUE : FALSE );
    }

    // KeWaitForSingleObject
    NTSTATUS ke_wait_for_single_object( void* object, KWAIT_REASON reason,
        KPROCESSOR_MODE mode, bool alertable, LARGE_INTEGER* timeout ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeWaitForSingleObject" ) );
            if ( !export_address ) return STATUS_NOT_FOUND;
        }
        using function_t = NTSTATUS( * )( void*, KWAIT_REASON, KPROCESSOR_MODE, BOOLEAN, LARGE_INTEGER* );
        return reinterpret_cast< function_t >( export_address )( object, reason, mode, alertable ? TRUE : FALSE, timeout );
    }

    // KeGetCurrentProcessorNumberEx
    std::uint32_t ke_get_current_processor_number_ex( processor_number_t* proc_number ) {
        static auto export_address = 0ull;
        if ( !export_address ) {
            export_address = get_export( m_ntoskrnl_base, oxorany( "KeGetCurrentProcessorNumberEx" ) );
            if ( !export_address ) return 0;
        }
        using function_t = std::uint32_t( * )( processor_number_t* );
        return reinterpret_cast< function_t >( export_address )( proc_number );
    }
}