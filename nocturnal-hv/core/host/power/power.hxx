#pragma once

namespace callback {
    void power_callback_routine(
        void* callback_context,
        void* argument1,
        void* argument2 ) {

        const void* po_cb_system_state_lock = ( void* )3;

        if ( argument1 != po_cb_system_state_lock )
            return;

        if ( argument2 != nullptr ) {
            kernel::dbg_print( "[nocturnal] system resuming from sleep\n" );
            utility::execute_on_each_processor( [ & ]( std::uint32_t idx ) -> bool {
                return guest::vmcb::virtualize_processor( idx );
                } );
        }
        else {
            kernel::dbg_print( "[nocturnal] system entering sleep\n" );
            utility::execute_on_each_processor( [ & ]( std::uint32_t idx ) -> bool {
                return guest::vmcb::devirtualize_processor( idx );
                } );
        }
    }

    bool create( ) {
        constexpr wchar_t callback_name[ ] = L"\\Callback\\PowerState";

        unicode_string_t name{ };
        name.m_buffer = const_cast< wchar_t* >( callback_name );
        name.m_length = sizeof( callback_name ) - sizeof( wchar_t );
        name.m_maximum_length = sizeof( callback_name );

        object_attributes_t attributes{ };
        attributes.m_length = sizeof( object_attributes_t );
        attributes.m_object_name = &name;
        attributes.m_attributes = 0x40;

        void* callback_object = nullptr;
        auto result = kernel::ex_create_callback(
            &callback_object,
            &attributes,
            false,
            true );

        if ( !result ) {
            kernel::dbg_print( "[nocturnal] could not open \\callback\\powerstate object\n" );
            return false;
        }

        guest::m_power_callback_registration = kernel::ex_register_callback(
            callback_object,
            reinterpret_cast< void* >( power_callback_routine ),
            nullptr );

        kernel::obf_dereference_object( callback_object );

        if ( !guest::m_power_callback_registration ) {
            kernel::dbg_print( "[nocturnal] could not register callback\n" );
            return false;
        }

        kernel::dbg_print( "[nocturnal] callback registered successfully\n" );
        return true;
    }

    void unload( ) {
        if ( !guest::m_power_callback_registration )
            return;

        kernel::ex_unregister_callback( guest::m_power_callback_registration );
        guest::m_power_callback_registration = nullptr;
        kernel::dbg_print( "[nocturnal] callback unregistered\n" );
    }
}