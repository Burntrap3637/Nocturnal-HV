namespace npt {

    // Matches SimpleSvm's SvBuildNestedPageTables exactly.
    // Maps 0 -> 512GB using 2MB large pages.
    // All allocations are physically contiguous and page-aligned.

    struct pml4_table_t {
        pml4_entry_2mb_t entries[ 512 ];
    };

    struct pdpt_table_t {
        pdpt_entry_2mb_t entries[ 512 ];
    };

    struct pd_table_t {
        pd_entry_2mb_t entries[ 512 ];
    };

    // Stored in shared_state so we can free them later
    // Replace your existing m_pml4_entries / m_pml4e_trees with these
    pml4_table_t* g_pml4 = nullptr;
    pdpt_table_t* g_pdpt = nullptr;  // one PDPT covers 512GB
    pd_table_t* g_pds = nullptr;  // 512 PDs, one per PDPT entry

    bool build_identity_map( ) {
        g_pml4 = memory::alloc_contiguous<pml4_table_t*>( sizeof( pml4_table_t ) );
        if ( !g_pml4 ) return false;

        for ( std::uint64_t pml4_idx = 0; pml4_idx < 2; pml4_idx++ ) {
            auto* pdpt = memory::alloc_contiguous<pdpt_table_t*>( sizeof( pdpt_table_t ) );
            if ( !pdpt ) return false;

            auto* pds = memory::alloc_contiguous<pd_table_t*>( sizeof( pd_table_t ) * 512 );
            if ( !pds ) return false;

            auto pdpt_pa = kernel::mm_get_physical_address( pdpt ).m_quad_part;
            g_pml4->entries[ pml4_idx ].m_page_frame_number = pdpt_pa >> 12;
            g_pml4->entries[ pml4_idx ].m_valid = 1;
            g_pml4->entries[ pml4_idx ].m_write = 1;
            g_pml4->entries[ pml4_idx ].m_user = 1;

            for ( std::uint64_t pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++ ) {
                auto pd_pa = kernel::mm_get_physical_address( &pds[ pdpt_idx ] ).m_quad_part;
                pdpt->entries[ pdpt_idx ].m_page_frame_number = pd_pa >> 12;
                pdpt->entries[ pdpt_idx ].m_valid = 1;
                pdpt->entries[ pdpt_idx ].m_write = 1;
                pdpt->entries[ pdpt_idx ].m_user = 1;

                for ( std::uint64_t pd_idx = 0; pd_idx < 512; pd_idx++ ) {
                    std::uint64_t phys =
                        ( pml4_idx * 512ULL * 512ULL + pdpt_idx * 512ULL + pd_idx )
                        * 0x200000ULL;
                    pds[ pdpt_idx ].entries[ pd_idx ].m_page_frame_number = phys >> 21;
                    pds[ pdpt_idx ].entries[ pd_idx ].m_valid = 1;
                    pds[ pdpt_idx ].entries[ pd_idx ].m_write = 1;
                    pds[ pdpt_idx ].entries[ pd_idx ].m_user = 1;
                    pds[ pdpt_idx ].entries[ pd_idx ].m_large_page = 1;
                }
            }
        }
        return true;
    }

    std::uint64_t get_pml4_pa( ) {
        return kernel::mm_get_physical_address( g_pml4 ).m_quad_part;
    }

    void build_msrpm( ) {
        constexpr std::uint32_t bits_per_msr = 2;
        constexpr std::uint32_t second_range_base = 0xc0000000;
        constexpr std::uint32_t second_msrpm_offset = 0x800 * 8;

        auto offset = second_msrpm_offset +
            ( ia32_msr_efer - second_range_base ) * bits_per_msr;

        auto bitmap = reinterpret_cast< std::uint8_t* >( m_shared_state->m_msrpm );

        // Intercept EFER writes only (bit offset+1)
        bitmap[ ( offset + 1 ) / 8 ] |= ( 1u << ( ( offset + 1 ) % 8 ) );
    }
}