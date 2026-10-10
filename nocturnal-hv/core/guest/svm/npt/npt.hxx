namespace npt {

    struct pml4_table_t {
        pml4_entry_2mb_t entries[ 512 ];
    };

    struct pdpt_table_t {
        pdpt_entry_2mb_t entries[ 512 ];
    };

    struct pd_table_t {
        pd_entry_2mb_t entries[ 512 ];
    };

    struct npt_pte_t {
        union {
            std::uint64_t m_as_uint64;
            struct {
                std::uint64_t m_valid      : 1;   // [0]
                std::uint64_t m_write      : 1;   // [1]
                std::uint64_t m_user       : 1;   // [2]
                std::uint64_t m_pwt        : 1;   // [3]
                std::uint64_t m_pcd        : 1;   // [4]
                std::uint64_t m_accessed   : 1;   // [5]
                std::uint64_t m_dirty      : 1;   // [6]
                std::uint64_t m_pat        : 1;   // [7]
                std::uint64_t m_global     : 1;   // [8]
                std::uint64_t m_avl        : 3;   // [9:11]
                std::uint64_t m_pfn        : 40;  // [12:51]
                std::uint64_t m_reserved   : 11;  // [52:62]
                std::uint64_t m_no_execute : 1;   // [63]
            };
        };
    };
    static_assert( sizeof( npt_pte_t ) == 8 );

    struct npt_pt_table_t {
        npt_pte_t entries[ 512 ];
    };

    struct split_entry_t {
        std::uint64_t   m_pd_gpa;  // 2MB-aligned GPA of the split region
        npt_pt_table_t* m_pt;      // host VA of the 4KB PT
    };

    pml4_table_t* g_pml4       = nullptr;
    pdpt_table_t* g_pdpt       = nullptr;
    pd_table_t*   g_pds        = nullptr;

    static constexpr std::uint32_t max_split_pages = 256;
    split_entry_t g_split_table[ max_split_pages ]{};
    std::uint32_t g_split_count = 0;

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

    pd_entry_2mb_t* find_pde( std::uint64_t gpa ) {
        if ( !g_pml4 ) return nullptr;

        const auto pml4_idx = ( gpa >> 39 ) & 0x1FF;
        const auto pdpt_idx = ( gpa >> 30 ) & 0x1FF;
        const auto pd_idx   = ( gpa >> 21 ) & 0x1FF;

        auto& pml4e = g_pml4->entries[ pml4_idx ];
        if ( !pml4e.m_valid ) return nullptr;

        auto* pdpt = reinterpret_cast< pdpt_table_t* >(
            physical::physical_to_virtual( pml4e.m_page_frame_number << 12 ) );
        if ( !pdpt ) return nullptr;

        auto& pdpte = pdpt->entries[ pdpt_idx ];
        if ( !pdpte.m_valid ) return nullptr;

        auto* pd = reinterpret_cast< pd_table_t* >(
            physical::physical_to_virtual( pdpte.m_page_frame_number << 12 ) );
        if ( !pd ) return nullptr;

        return &pd->entries[ pd_idx ];
    }

    bool split_page( std::uint64_t gpa ) {
        const std::uint64_t aligned_2mb = gpa & ~0x1FFFFFull;

        for ( std::uint32_t i = 0; i < g_split_count; i++ ) {
            if ( g_split_table[ i ].m_pd_gpa == aligned_2mb )
                return true;
        }

        if ( g_split_count >= max_split_pages ) return false;

        auto* pde = find_pde( aligned_2mb );
        if ( !pde || !pde->m_valid || !pde->m_large_page ) return false;

        auto* pt = memory::alloc_contiguous<npt_pt_table_t*>( sizeof( npt_pt_table_t ) );
        if ( !pt ) return false;

        const std::uint64_t base_pa = static_cast< std::uint64_t >( pde->m_page_frame_number ) << 21;
        for ( std::uint32_t i = 0; i < 512; i++ ) {
            pt->entries[ i ].m_as_uint64 = 0;
            pt->entries[ i ].m_valid     = 1;
            pt->entries[ i ].m_write     = 1;
            pt->entries[ i ].m_user      = 1;
            pt->entries[ i ].m_pfn       = ( base_pa >> 12 ) + i;
        }

        const auto pt_pa = kernel::mm_get_physical_address( pt ).m_quad_part;
        pde->m_as_uint64 = 0x7ull | ( pt_pa & ~0xFFFull );

        g_split_table[ g_split_count ].m_pd_gpa = aligned_2mb;
        g_split_table[ g_split_count ].m_pt     = pt;
        g_split_count++;

        return true;
    }

    npt_pte_t* find_pte( std::uint64_t gpa ) {
        const std::uint64_t aligned_2mb = gpa & ~0x1FFFFFull;
        for ( std::uint32_t i = 0; i < g_split_count; i++ ) {
            if ( g_split_table[ i ].m_pd_gpa == aligned_2mb ) {
                const auto pt_idx = ( gpa >> 12 ) & 0x1FF;
                return &g_split_table[ i ].m_pt->entries[ pt_idx ];
            }
        }
        return nullptr;
    }

    void set_nx( std::uint64_t gpa, bool nx ) {
        auto* pte = find_pte( gpa );
        if ( pte ) pte->m_no_execute = nx ? 1u : 0u;
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