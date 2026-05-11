#pragma once

typedef union _virt_addr_t {
    std::uintptr_t value;
    struct {
        std::uint64_t offset : 12;        // 0:11
        std::uint64_t pte_index : 9;      // 12:20
        std::uint64_t pde_index : 9;      // 21:29
        std::uint64_t pdpte_index : 9;    // 30:38
        std::uint64_t pml4e_index : 9;    // 39:47
        std::uint64_t reserved : 16;      // 48:63
    };
    struct {
        std::uint64_t offset_4kb : 12;    // 4KB page offset
        std::uint64_t pt_offset : 9;
        std::uint64_t pd_offset : 9;
        std::uint64_t pdpt_offset : 9;
        std::uint64_t pml4_offset : 9;
        std::uint64_t reserved2 : 16;
    };
    struct {
        std::uint64_t offset_2mb : 21;    // 2MB page offset
        std::uint64_t pd_offset2 : 9;
        std::uint64_t pdpt_offset2 : 9;
        std::uint64_t pml4_offset2 : 9;
        std::uint64_t reserved3 : 16;
    };
    struct {
        std::uint64_t offset_1gb : 30;    // 1GB page offset
        std::uint64_t pdpt_offset3 : 9;
        std::uint64_t pml4_offset3 : 9;
        std::uint64_t reserved4 : 16;
    };
} virt_addr_t, * pvirt_addr_t;

typedef union _pml4e {
    std::uint64_t value;
    struct {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t ignored0 : 1;                  // Ignored
        std::uint64_t large_page : 1;               // Reserved (must be 0)
        std::uint64_t ignored1 : 4;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pml4e, * ppml4e;

typedef union _pdpte {
    std::uint64_t value;
    struct {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to (1GB pages)
        std::uint64_t page_size : 1;                // 1=1GB page, 0=points to page directory
        std::uint64_t ignored1 : 4;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pdpte, * ppdpte;

typedef union _pde {
    std::uint64_t value;
    struct {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to (2MB pages)
        std::uint64_t page_size : 1;                // 1=2MB page, 0=points to page table
        std::uint64_t global : 1;                   // Global page (if CR4.PGE=1)
        std::uint64_t ignored1 : 3;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pde, * ppde;

typedef union _pte {
    std::uint64_t value;
    struct {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to
        std::uint64_t pat : 1;                      // Page Attribute Table bit
        std::uint64_t global : 1;                   // Global page
        std::uint64_t ignored1 : 3;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 7;                 // Ignored
        std::uint64_t protection_key : 4;           // Protection key
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pte, * ppte;

struct physical_address_t {
    union {
        struct {
            std::uint32_t m_low_part;      // +0x000
            std::int32_t m_high_part;      // +0x004
        };
        struct {
            std::uint64_t m_quad_part;     // +0x000
        };
    };
}; // Size: 0x008

struct physical_memory_range_t {
    ularge_integer_t m_base_page;
    ularge_integer_t m_page_count;
}; // Size: 0x010

struct processor_number_t {
    std::uint16_t m_group;
    std::uint8_t  m_number;
    std::uint8_t  m_reserved;
};

typedef union segment_selector_t {
    std::uint16_t value;

    struct {
        std::uint16_t rpl : 2;
        std::uint16_t ti : 1;
        std::uint16_t index : 13;
    };
};

#pragma pack(push, 1)
struct descriptor_table_register_t {
    std::uint16_t m_limit;
    std::uint64_t m_base;
};
static_assert( sizeof( descriptor_table_register_t ) == 10 );
#pragma pack(pop)

typedef descriptor_table_register_t gdtr_t;
typedef descriptor_table_register_t idtr_t;

struct segment_descriptor_t {
    std::uint16_t m_limit_low;
    std::uint16_t m_base_low;
    union {
        struct {
            std::uint32_t m_base_middle : 8;
            std::uint32_t m_type : 4;
            std::uint32_t m_system : 1;
            std::uint32_t m_dpl : 2;
            std::uint32_t m_present : 1;
            std::uint32_t m_limit_high : 4;
            std::uint32_t m_avl : 1;
            std::uint32_t m_long_mode : 1;
            std::uint32_t m_default_bit : 1;
            std::uint32_t m_granularity : 1;
            std::uint32_t m_base_high : 8;
        };
        std::uint32_t m_as_uint32;
    };
};
static_assert( sizeof( segment_descriptor_t ) == 8 );

struct segment_attribute_t {
    union {
        std::uint16_t m_as_uint16;
        struct {
            std::uint16_t m_type : 4;
            std::uint16_t m_system : 1;
            std::uint16_t m_dpl : 2;
            std::uint16_t m_present : 1;
            std::uint16_t m_avl : 1;
            std::uint16_t m_long_mode : 1;
            std::uint16_t m_default_bit : 1;
            std::uint16_t m_granularity : 1;
            std::uint16_t m_reserved1 : 4;
        };
    };
};
static_assert( sizeof( segment_attribute_t ) == 2 );

struct object_attributes_t {
    std::uint32_t m_length;
    void* m_root_directory;
    unicode_string_t* m_object_name;
    std::uint32_t m_attributes;
    void* m_security_descriptor;
    void* m_security_q1uality_of_service;
};