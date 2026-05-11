#pragma once

namespace utility {
    std::uint16_t get_access_right( std::uint16_t selector, std::uint64_t gdt_base ) {
        auto descriptor = reinterpret_cast< segment_descriptor_t* >(
            gdt_base + ( selector & ~rpl_mask ) );

        segment_attribute_t attribute{ };
        attribute.m_type = descriptor->m_type;
        attribute.m_system = descriptor->m_system;
        attribute.m_dpl = descriptor->m_dpl;
        attribute.m_present = descriptor->m_present;
        attribute.m_avl = descriptor->m_avl;
        attribute.m_long_mode = descriptor->m_long_mode;
        attribute.m_default_bit = descriptor->m_default_bit;
        attribute.m_granularity = descriptor->m_granularity;
        attribute.m_reserved1 = 0;

        return attribute.m_as_uint16;
    }

    template <typename func_t>
    bool execute_on_each_processor( func_t callback, std::uint32_t* processors = nullptr ) {
        std::uint64_t processed = 0;
        for ( auto idx = 0; idx < kernel::ke_query_active_processor_count( ); idx++ ) {
            processor_number_t processor_number{ };
            if ( !kernel::ke_get_processor_number_from_index( idx, &processor_number ) )
                return false;

            group_affinity_t affinity{ };
            affinity.m_group = processor_number.m_group;
            affinity.m_mask = 1ull << processor_number.m_number;

            group_affinity_t old_affinity{ };
            kernel::ke_set_system_group_affinity_thread( &affinity, &old_affinity );
            auto result = callback( idx );
            kernel::ke_revert_to_user_group_affinity_thread( &old_affinity );

            if ( processors )
                *processors = ++processed;
            if ( !result )
                return false;
        }

        return true;
    }

}