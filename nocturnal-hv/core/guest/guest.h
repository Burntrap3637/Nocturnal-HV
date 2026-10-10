#pragma once
constexpr std::uint32_t kernel_stack_size = 0x6000;

namespace guest {

    void* m_power_callback_registration;

    struct pml4e_tree_t {
        __declspec( align( 0x1000 ) ) pdpt_entry_2mb_t m_pdpt_entries[ 512 ];
        __declspec( align( 0x1000 ) ) pd_entry_2mb_t   m_pd_entries[ 512 ][ 512 ];
    };

    struct watched_page_t {
        std::uint64_t m_guest_pa;
        std::uint8_t* m_shadow;   // 4KB, ExAllocatePool2-allocated
        bool          m_captured;
        bool          m_active;
    };

    struct shared_state_t {
        void* m_msrpm;
        pml4_entry_2mb_t* m_pml4_entries;
        pml4e_tree_t* m_pml4e_trees[ 2 ];
        std::uint32_t     m_nrip_save;
        std::uint32_t     m_decode_assists;
        watched_page_t    m_watched_pages[ 32 ];
        std::uint32_t     m_watched_count;
    };

    struct host_stack_layout_t {
        std::uint64_t guest_vmcb_pa;   // +0x00
        std::uint64_t host_vmcb_pa;    // +0x08
        void* self;            // +0x10
        void* shared_vp_data;  // +0x18
        std::uint64_t reserved1;       // +0x20
    };
    static_assert( sizeof( host_stack_layout_t ) == 0x28 );

    struct virtual_processor_data_t {
        union {
            __declspec( align( PAGE_SIZE ) ) std::uint8_t m_host_stack[ KERNEL_STACK_SIZE ];
            struct {
                std::uint8_t stack_contents[ KERNEL_STACK_SIZE - ( sizeof( void* ) * 6 ) - sizeof( KTRAP_FRAME ) ];
                KTRAP_FRAME trap_frame;
                std::uint64_t guest_vmcb_pa;
                std::uint64_t host_vmcb_pa;
                virtual_processor_data_t* self;
                shared_state_t* shared_vp_data;
                std::uint64_t padding1;
                std::uint64_t reserved1;
            } m_host_stack_layout;
        };
        __declspec( align( PAGE_SIZE ) ) vmcb_t m_guest_vmcb;
        __declspec( align( PAGE_SIZE ) ) vmcb_t m_host_vmcb;
        __declspec( align( PAGE_SIZE ) ) std::uint8_t m_host_state_area[ PAGE_SIZE ];
    };
    // offset of `self` from the base of the host stack (guest_vmcb_pa)
    constexpr std::uint32_t host_stack_self_offset = offsetof(
        guest::virtual_processor_data_t,
        m_host_stack_layout.self )
        - offsetof(
            guest::virtual_processor_data_t,
            m_host_stack_layout.guest_vmcb_pa );

    static_assert( host_stack_self_offset == 0x10 );
} // namespace guest

inline guest::shared_state_t* m_shared_state = nullptr;
inline guest::virtual_processor_data_t** m_processor_states = nullptr;
inline std::uint32_t                     m_processor_count = 0;