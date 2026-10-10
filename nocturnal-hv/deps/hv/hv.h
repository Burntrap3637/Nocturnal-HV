#pragma once
#define ia32_msr_pat    0x00000277
#define ia32_msr_efer   0xc0000080
#define efer_svme       (1UL << 12)

constexpr std::uint16_t dpl_system = 0;
#define rpl_mask 3

 //
 // A size of two the MSR permissions map.
 //
static constexpr std::uint32_t svm_msr_permissions_map_size = ( 0x1000 * 2 );

//
// See "SVM Related MSRs"
//
static constexpr std::uint32_t svm_msr_vm_cr = 0xc0010114;
static constexpr std::uint32_t svm_msr_vm_hsave_pa = 0xc0010117;
static constexpr std::uint64_t svm_vm_cr_svmdis = ( 1UL << 4 );

// CPUID 0x8000000A EDX SVM feature bits
constexpr std::uint32_t cpuid_edx_nrip_save = ( 1u << 3 );  // Next-RIP saved on #VMEXIT
constexpr std::uint32_t cpuid_edx_decode_assists = ( 1u << 7 );  // Exit info pre-decoded

// cpuid leaves
constexpr std::uint32_t cpuid_max_standard_fn = 0x00000000;
constexpr std::uint32_t cpuid_processor_features = 0x00000001;
constexpr std::uint32_t cpuid_processor_features_ex = 0x80000001;
constexpr std::uint32_t cpuid_svm_features = 0x8000000a;
constexpr std::uint32_t cpuid_hv_vendor = 0x40000000;
constexpr std::uint32_t cpuid_hv_interface = 0x40000001;
constexpr std::uint32_t cpuid_unload = 0x68766368;

// cpuid feature bits
constexpr std::uint32_t cpuid_ecx_svm = ( 1u << 2 );  // ecx bit 2  - svm support
constexpr std::uint32_t cpuid_ecx_hypervisor_present = ( 1u << 31 );  // ecx bit 31 - hypervisor present
constexpr std::uint32_t cpuid_edx_np = ( 1u << 0 );  // edx bit 0  - nested paging

//
// See "VMCB Layout, Control Area"
//
static constexpr std::uint32_t svm_intercept_misc1_cpuid = ( 1UL << 18 );
static constexpr std::uint32_t svm_intercept_misc1_msr_prot = ( 1UL << 28 );
static constexpr std::uint32_t svm_intercept_misc1_xsetbv = ( 1UL << 27 );
static constexpr std::uint32_t svm_intercept_misc2_vmrun = ( 1UL << 0 );
static constexpr std::uint32_t svm_intercept_misc2_vmmcall = ( 1UL << 1 );
static constexpr std::uint32_t svm_intercept_misc2_vmload = ( 1UL << 2 );
static constexpr std::uint32_t svm_intercept_misc2_vmsave = ( 1UL << 3 );
static constexpr std::uint32_t svm_intercept_misc2_stgi = ( 1UL << 4 );
static constexpr std::uint32_t svm_intercept_misc2_clgi = ( 1UL << 5 );
static constexpr std::uint32_t svm_intercept_misc2_skinit = ( 1UL << 6 );
static constexpr std::uint32_t svm_intercept_misc2_rdtscp = ( 1UL << 7 );
static constexpr std::uint32_t svm_intercept_misc2_icebp = ( 1UL << 8 );
static constexpr std::uint32_t svm_intercept_misc2_wbinvd = ( 1UL << 9 );
static constexpr std::uint32_t svm_intercept_misc2_monitor = ( 1UL << 10 );
static constexpr std::uint32_t svm_intercept_misc2_mwait = ( 1UL << 11 );
static constexpr std::uint32_t svm_intercept_misc2_mwait_armed = ( 1UL << 12 );
static constexpr std::uint32_t svm_intercept_misc2_xsetbv = ( 1UL << 13 );
static constexpr std::uint32_t svm_intercept_misc2_rdpru = ( 1UL << 14 );
static constexpr std::uint32_t svm_intercept_misc2_efer_write_after_done = ( 1UL << 15 );
static constexpr std::uint32_t svm_intercept_misc2_cr0_write_after_done = ( 1UL << 16 );
static constexpr std::uint32_t svm_intercept_misc2_cr1_write_after_done = ( 1UL << 17 );
static constexpr std::uint32_t svm_intercept_misc2_cr2_write_after_done = ( 1UL << 18 );
static constexpr std::uint32_t svm_intercept_misc2_cr3_write_after_done = ( 1UL << 19 );
static constexpr std::uint32_t svm_intercept_misc2_cr4_write_after_done = ( 1UL << 20 );
static constexpr std::uint32_t svm_intercept_misc2_cr5_write_after_done = ( 1UL << 21 );
static constexpr std::uint32_t svm_intercept_misc2_cr6_write_after_done = ( 1UL << 22 );
static constexpr std::uint32_t svm_intercept_misc2_cr7_write_after_done = ( 1UL << 23 );
static constexpr std::uint32_t svm_intercept_misc2_cr8_write_after_done = ( 1UL << 24 );
static constexpr std::uint32_t svm_intercept_misc2_nrip_save = ( 1UL << 3 );
static constexpr std::uint64_t svm_np_enable_np_enable = ( 1ULL << 0 );

// VMCB clean bits (AMD APM Vol 2 §15.15.2)
static constexpr std::uint32_t vmcb_clean_np            = ( 1u << 4 );   // nested paging state (n_cr3, np_enable)
// TLB control values (AMD APM Vol 2 §15.16)
static constexpr std::uint32_t tlb_ctrl_flush_asid      = 3;             // flush all TLB entries for this guest ASID
// NPF EXITINFO1 bits (AMD APM Vol 2 §15.25.6)
static constexpr std::uint64_t npf_error_present        = ( 1ull << 0 ); // physical page present in NPT
static constexpr std::uint64_t npf_error_write          = ( 1ull << 1 ); // write access
static constexpr std::uint64_t npf_error_user           = ( 1ull << 2 ); // user-mode access
static constexpr std::uint64_t npf_error_rsvd           = ( 1ull << 3 ); // reserved bit violation
static constexpr std::uint64_t npf_error_fetch          = ( 1ull << 4 ); // instruction fetch (NX violation)
// CPUID hypercall leaves
constexpr std::uint32_t cpuid_watch_page                = 0x77617463;    // 'watc' — ECX=GPA>>12
constexpr std::uint32_t cpuid_read_page                 = 0x72656164;    // 'read' — ECX=GPA>>12, RDX:RBX=dest VA

#pragma pack(push, 1)
struct vmcb_control_area_t {
    std::uint16_t m_intercept_cr_read;              // +0x000
    std::uint16_t m_intercept_cr_write;             // +0x002
    std::uint16_t m_intercept_dr_read;              // +0x004
    std::uint16_t m_intercept_dr_write;             // +0x006
    std::uint32_t m_intercept_exception;            // +0x008
    std::uint32_t m_intercept_misc1;                // +0x00c
    std::uint32_t m_intercept_misc2;                // +0x010
    std::uint8_t  m_reserved1[ 0x03c - 0x014 ];    // +0x014
    std::uint16_t m_pause_filter_threshold;         // +0x03c
    std::uint16_t m_pause_filter_count;             // +0x03e
    std::uint64_t m_iopm_base_pa;                   // +0x040
    std::uint64_t m_msrpm_base_pa;                  // +0x048
    std::uint64_t m_tsc_offset;                     // +0x050
    std::uint32_t m_guest_asid;                     // +0x058
    std::uint32_t m_tlb_control;                    // +0x05c
    std::uint64_t m_v_intr;                         // +0x060
    std::uint64_t m_interrupt_shadow;               // +0x068
    std::uint64_t m_exit_code;                      // +0x070
    std::uint64_t m_exit_info1;                     // +0x078
    std::uint64_t m_exit_info2;                     // +0x080
    std::uint64_t m_exit_int_info;                  // +0x088
    std::uint64_t m_np_enable;                      // +0x090
    std::uint64_t m_avic_apic_bar;                  // +0x098
    std::uint64_t m_guest_pa_of_ghcb;               // +0x0a0
    std::uint64_t m_event_inj;                      // +0x0a8
    std::uint64_t m_n_cr3;                          // +0x0b0
    std::uint64_t m_lbr_virtualization_enable;      // +0x0b8
    std::uint64_t m_vmcb_clean;                     // +0x0c0
    std::uint64_t m_n_rip;                          // +0x0c8
    std::uint8_t  m_num_of_bytes_fetched;           // +0x0d0
    std::uint8_t  m_guest_instruction_bytes[ 15 ];  // +0x0d1
    std::uint64_t m_avic_apic_backing_page_pointer; // +0x0e0
    std::uint64_t m_reserved2;                      // +0x0e8
    std::uint64_t m_avic_logical_table_pointer;     // +0x0f0
    std::uint64_t m_avic_physical_table_pointer;    // +0x0f8
    std::uint64_t m_reserved3;                      // +0x100
    std::uint64_t m_vmcb_save_state_pointer;        // +0x108
    std::uint8_t  m_reserved4[ 0x400 - 0x110 ];    // +0x110
};
static_assert( sizeof( vmcb_control_area_t ) == 0x400 );
static_assert( offsetof( vmcb_control_area_t, m_exit_code ) == 0x070, "exit_code offset wrong" );
static_assert( offsetof( vmcb_control_area_t, m_n_rip ) == 0x0c8, "nRIP offset wrong" );
static_assert( offsetof( vmcb_control_area_t, m_exit_info1 ) == 0x078, "exit_info1 offset wrong" );

struct vmcb_state_save_area_t {
    std::uint16_t m_es_selector;                    // +0x000
    std::uint16_t m_es_attrib;                      // +0x002
    std::uint32_t m_es_limit;                       // +0x004
    std::uint64_t m_es_base;                        // +0x008
    std::uint16_t m_cs_selector;                    // +0x010
    std::uint16_t m_cs_attrib;                      // +0x012
    std::uint32_t m_cs_limit;                       // +0x014
    std::uint64_t m_cs_base;                        // +0x018
    std::uint16_t m_ss_selector;                    // +0x020
    std::uint16_t m_ss_attrib;                      // +0x022
    std::uint32_t m_ss_limit;                       // +0x024
    std::uint64_t m_ss_base;                        // +0x028
    std::uint16_t m_ds_selector;                    // +0x030
    std::uint16_t m_ds_attrib;                      // +0x032
    std::uint32_t m_ds_limit;                       // +0x034
    std::uint64_t m_ds_base;                        // +0x038
    std::uint16_t m_fs_selector;                    // +0x040
    std::uint16_t m_fs_attrib;                      // +0x042
    std::uint32_t m_fs_limit;                       // +0x044
    std::uint64_t m_fs_base;                        // +0x048
    std::uint16_t m_gs_selector;                    // +0x050
    std::uint16_t m_gs_attrib;                      // +0x052
    std::uint32_t m_gs_limit;                       // +0x054
    std::uint64_t m_gs_base;                        // +0x058
    std::uint16_t m_gdtr_selector;                  // +0x060
    std::uint16_t m_gdtr_attrib;                    // +0x062
    std::uint32_t m_gdtr_limit;                     // +0x064
    std::uint64_t m_gdtr_base;                      // +0x068
    std::uint16_t m_ldtr_selector;                  // +0x070
    std::uint16_t m_ldtr_attrib;                    // +0x072
    std::uint32_t m_ldtr_limit;                     // +0x074
    std::uint64_t m_ldtr_base;                      // +0x078
    std::uint16_t m_idtr_selector;                  // +0x080
    std::uint16_t m_idtr_attrib;                    // +0x082
    std::uint32_t m_idtr_limit;                     // +0x084
    std::uint64_t m_idtr_base;                      // +0x088
    std::uint16_t m_tr_selector;                    // +0x090
    std::uint16_t m_tr_attrib;                      // +0x092
    std::uint32_t m_tr_limit;                       // +0x094
    std::uint64_t m_tr_base;                        // +0x098
    std::uint8_t  m_reserved1[ 0x0cb - 0x0a0 ];    // +0x0a0
    std::uint8_t  m_cpl;                            // +0x0cb
    std::uint32_t m_reserved2;                      // +0x0cc
    std::uint64_t m_efer;                           // +0x0d0
    std::uint8_t  m_reserved3[ 0x148 - 0x0d8 ];    // +0x0d8
    std::uint64_t m_cr4;                            // +0x148
    std::uint64_t m_cr3;                            // +0x150
    std::uint64_t m_cr0;                            // +0x158
    std::uint64_t m_dr7;                            // +0x160
    std::uint64_t m_dr6;                            // +0x168
    std::uint64_t m_rflags;                         // +0x170
    std::uint64_t m_rip;                            // +0x178
    std::uint8_t  m_reserved4[ 0x1d8 - 0x180 ];    // +0x180
    std::uint64_t m_rsp;                            // +0x1d8
    std::uint8_t  m_reserved5[ 0x1f8 - 0x1e0 ];    // +0x1e0
    std::uint64_t m_rax;                            // +0x1f8
    std::uint64_t m_star;                           // +0x200
    std::uint64_t m_l_star;                         // +0x208
    std::uint64_t m_c_star;                         // +0x210
    std::uint64_t m_sf_mask;                        // +0x218
    std::uint64_t m_kernel_gs_base;                 // +0x220
    std::uint64_t m_sysenter_cs;                    // +0x228
    std::uint64_t m_sysenter_esp;                   // +0x230
    std::uint64_t m_sysenter_eip;                   // +0x238
    std::uint64_t m_cr2;                            // +0x240
    std::uint8_t  m_reserved6[ 0x268 - 0x248 ];    // +0x248
    std::uint64_t m_g_pat;                          // +0x268
    std::uint64_t m_dbg_ctl;                        // +0x270
    std::uint64_t m_br_from;                        // +0x278
    std::uint64_t m_br_to;                          // +0x280
    std::uint64_t m_last_excep_from;                // +0x288
    std::uint64_t m_last_excep_to;                  // +0x290
};
static_assert( sizeof( vmcb_state_save_area_t ) == 0x298 );

struct vmcb_t {
    vmcb_control_area_t    m_control_area;
    vmcb_state_save_area_t m_state_save_area;
    std::uint8_t           m_reserved1[ 0x1000 - sizeof( vmcb_control_area_t ) - sizeof( vmcb_state_save_area_t ) ];
};
static_assert( sizeof( vmcb_t ) == 0x1000 );
#pragma pack(pop)

//
// See "Event Injection"
//
struct event_inj_t {
    union {
        std::uint64_t as_uint64;
        struct {
            std::uint64_t vector : 8;  // [0:7]
            std::uint64_t type : 3;  // [8:10]
            std::uint64_t error_code_valid : 1;  // [11]
            std::uint64_t reserved1 : 19; // [12:30]
            std::uint64_t valid : 1;  // [31]
            std::uint64_t error_code : 32; // [32:63]
        } fields;
    };
};
static_assert( sizeof( event_inj_t ) == 8,
    "event_inj size mismatch" );

#pragma pack(push, 1)
struct pml4_entry_2mb_t {
    union {
        std::uint64_t m_as_uint64;
        struct {
            std::uint64_t m_valid : 1;   // [0]
            std::uint64_t m_write : 1;   // [1]
            std::uint64_t m_user : 1;   // [2]
            std::uint64_t m_write_through : 1;   // [3]
            std::uint64_t m_cache_disable : 1;   // [4]
            std::uint64_t m_accessed : 1;   // [5]
            std::uint64_t m_reserved1 : 3;   // [6:8]
            std::uint64_t m_avl : 3;   // [9:11]
            std::uint64_t m_page_frame_number : 40;  // [12:51]
            std::uint64_t m_reserved2 : 11;  // [52:62]
            std::uint64_t m_no_execute : 1;   // [63]
        };
    };
};
static_assert( sizeof( pml4_entry_2mb_t ) == 8, "pml4_entry_2mb_t size mismatch" );

using pdpt_entry_2mb_t = pml4_entry_2mb_t;

struct pd_entry_2mb_t {
    union {
        std::uint64_t m_as_uint64;
        struct {
            std::uint64_t m_valid : 1;   // [0]
            std::uint64_t m_write : 1;   // [1]
            std::uint64_t m_user : 1;   // [2]
            std::uint64_t m_write_through : 1;   // [3]
            std::uint64_t m_cache_disable : 1;   // [4]
            std::uint64_t m_accessed : 1;   // [5]
            std::uint64_t m_dirty : 1;   // [6]
            std::uint64_t m_large_page : 1;   // [7]  must be 1 for 2mb
            std::uint64_t m_global : 1;   // [8]
            std::uint64_t m_avl : 3;   // [9:11]
            std::uint64_t m_pat : 1;   // [12]
            std::uint64_t m_reserved1 : 8;   // [13:20]
            std::uint64_t m_page_frame_number : 31;  // [21:51]
            std::uint64_t m_reserved2 : 11;  // [52:62]
            std::uint64_t m_no_execute : 1;   // [63]
        };
    };
};
static_assert( sizeof( pd_entry_2mb_t ) == 8, "pd_entry_2mb_t size mismatch" );
#pragma pack(pop)

struct guest_registers_t {
    std::uint64_t m_r15;
    std::uint64_t m_r14;
    std::uint64_t m_r13;
    std::uint64_t m_r12;
    std::uint64_t m_r11;
    std::uint64_t m_r10;
    std::uint64_t m_r9;
    std::uint64_t m_r8;
    std::uint64_t m_rdi;
    std::uint64_t m_rsi;
    std::uint64_t m_rbp;
    std::uint64_t m_rsp;   // ← the push -1 slot
    std::uint64_t m_rbx;
    std::uint64_t m_rdx;
    std::uint64_t m_rcx;
    std::uint64_t m_rax;
};
struct guest_context_t {
    guest_registers_t* m_vp_regs;
    bool               m_exit_vm;
};

//
// See "SVM Intercept Codes"
//
static constexpr std::int32_t vmexit_cr0_read = 0x0000;
static constexpr std::int32_t vmexit_cr1_read = 0x0001;
static constexpr std::int32_t vmexit_cr2_read = 0x0002;
static constexpr std::int32_t vmexit_cr3_read = 0x0003;
static constexpr std::int32_t vmexit_cr4_read = 0x0004;
static constexpr std::int32_t vmexit_cr5_read = 0x0005;
static constexpr std::int32_t vmexit_cr6_read = 0x0006;
static constexpr std::int32_t vmexit_cr7_read = 0x0007;
static constexpr std::int32_t vmexit_cr8_read = 0x0008;
static constexpr std::int32_t vmexit_cr9_read = 0x0009;
static constexpr std::int32_t vmexit_cr10_read = 0x000a;
static constexpr std::int32_t vmexit_cr11_read = 0x000b;
static constexpr std::int32_t vmexit_cr12_read = 0x000c;
static constexpr std::int32_t vmexit_cr13_read = 0x000d;
static constexpr std::int32_t vmexit_cr14_read = 0x000e;
static constexpr std::int32_t vmexit_cr15_read = 0x000f;
static constexpr std::int32_t vmexit_cr0_write = 0x0010;
static constexpr std::int32_t vmexit_cr1_write = 0x0011;
static constexpr std::int32_t vmexit_cr2_write = 0x0012;
static constexpr std::int32_t vmexit_cr3_write = 0x0013;
static constexpr std::int32_t vmexit_cr4_write = 0x0014;
static constexpr std::int32_t vmexit_cr5_write = 0x0015;
static constexpr std::int32_t vmexit_cr6_write = 0x0016;
static constexpr std::int32_t vmexit_cr7_write = 0x0017;
static constexpr std::int32_t vmexit_cr8_write = 0x0018;
static constexpr std::int32_t vmexit_cr9_write = 0x0019;
static constexpr std::int32_t vmexit_cr10_write = 0x001a;
static constexpr std::int32_t vmexit_cr11_write = 0x001b;
static constexpr std::int32_t vmexit_cr12_write = 0x001c;
static constexpr std::int32_t vmexit_cr13_write = 0x001d;
static constexpr std::int32_t vmexit_cr14_write = 0x001e;
static constexpr std::int32_t vmexit_cr15_write = 0x001f;
static constexpr std::int32_t vmexit_dr0_read = 0x0020;
static constexpr std::int32_t vmexit_dr1_read = 0x0021;
static constexpr std::int32_t vmexit_dr2_read = 0x0022;
static constexpr std::int32_t vmexit_dr3_read = 0x0023;
static constexpr std::int32_t vmexit_dr4_read = 0x0024;
static constexpr std::int32_t vmexit_dr5_read = 0x0025;
static constexpr std::int32_t vmexit_dr6_read = 0x0026;
static constexpr std::int32_t vmexit_dr7_read = 0x0027;
static constexpr std::int32_t vmexit_dr8_read = 0x0028;
static constexpr std::int32_t vmexit_dr9_read = 0x0029;
static constexpr std::int32_t vmexit_dr10_read = 0x002a;
static constexpr std::int32_t vmexit_dr11_read = 0x002b;
static constexpr std::int32_t vmexit_dr12_read = 0x002c;
static constexpr std::int32_t vmexit_dr13_read = 0x002d;
static constexpr std::int32_t vmexit_dr14_read = 0x002e;
static constexpr std::int32_t vmexit_dr15_read = 0x002f;
static constexpr std::int32_t vmexit_dr0_write = 0x0030;
static constexpr std::int32_t vmexit_dr1_write = 0x0031;
static constexpr std::int32_t vmexit_dr2_write = 0x0032;
static constexpr std::int32_t vmexit_dr3_write = 0x0033;
static constexpr std::int32_t vmexit_dr4_write = 0x0034;
static constexpr std::int32_t vmexit_dr5_write = 0x0035;
static constexpr std::int32_t vmexit_dr6_write = 0x0036;
static constexpr std::int32_t vmexit_dr7_write = 0x0037;
static constexpr std::int32_t vmexit_dr8_write = 0x0038;
static constexpr std::int32_t vmexit_dr9_write = 0x0039;
static constexpr std::int32_t vmexit_dr10_write = 0x003a;
static constexpr std::int32_t vmexit_dr11_write = 0x003b;
static constexpr std::int32_t vmexit_dr12_write = 0x003c;
static constexpr std::int32_t vmexit_dr13_write = 0x003d;
static constexpr std::int32_t vmexit_dr14_write = 0x003e;
static constexpr std::int32_t vmexit_dr15_write = 0x003f;
static constexpr std::int32_t vmexit_exception_de = 0x0040;
static constexpr std::int32_t vmexit_exception_db = 0x0041;
static constexpr std::int32_t vmexit_exception_nmi = 0x0042;
static constexpr std::int32_t vmexit_exception_bp = 0x0043;
static constexpr std::int32_t vmexit_exception_of = 0x0044;
static constexpr std::int32_t vmexit_exception_br = 0x0045;
static constexpr std::int32_t vmexit_exception_ud = 0x0046;
static constexpr std::int32_t vmexit_exception_nm = 0x0047;
static constexpr std::int32_t vmexit_exception_df = 0x0048;
static constexpr std::int32_t vmexit_exception_09 = 0x0049;
static constexpr std::int32_t vmexit_exception_ts = 0x004a;
static constexpr std::int32_t vmexit_exception_np = 0x004b;
static constexpr std::int32_t vmexit_exception_ss = 0x004c;
static constexpr std::int32_t vmexit_exception_gp = 0x004d;
static constexpr std::int32_t vmexit_exception_pf = 0x004e;
static constexpr std::int32_t vmexit_exception_15 = 0x004f;
static constexpr std::int32_t vmexit_exception_mf = 0x0050;
static constexpr std::int32_t vmexit_exception_ac = 0x0051;
static constexpr std::int32_t vmexit_exception_mc = 0x0052;
static constexpr std::int32_t vmexit_exception_xf = 0x0053;
static constexpr std::int32_t vmexit_exception_20 = 0x0054;
static constexpr std::int32_t vmexit_exception_21 = 0x0055;
static constexpr std::int32_t vmexit_exception_22 = 0x0056;
static constexpr std::int32_t vmexit_exception_23 = 0x0057;
static constexpr std::int32_t vmexit_exception_24 = 0x0058;
static constexpr std::int32_t vmexit_exception_25 = 0x0059;
static constexpr std::int32_t vmexit_exception_26 = 0x005a;
static constexpr std::int32_t vmexit_exception_27 = 0x005b;
static constexpr std::int32_t vmexit_exception_28 = 0x005c;
static constexpr std::int32_t vmexit_exception_vc = 0x005d;
static constexpr std::int32_t vmexit_exception_sx = 0x005e;
static constexpr std::int32_t vmexit_exception_31 = 0x005f;
static constexpr std::int32_t vmexit_intr = 0x0060;
static constexpr std::int32_t vmexit_nmi = 0x0061;
static constexpr std::int32_t vmexit_smi = 0x0062;
static constexpr std::int32_t vmexit_init = 0x0063;
static constexpr std::int32_t vmexit_vintr = 0x0064;
static constexpr std::int32_t vmexit_cr0_sel_write = 0x0065;
static constexpr std::int32_t vmexit_idtr_read = 0x0066;
static constexpr std::int32_t vmexit_gdtr_read = 0x0067;
static constexpr std::int32_t vmexit_ldtr_read = 0x0068;
static constexpr std::int32_t vmexit_tr_read = 0x0069;
static constexpr std::int32_t vmexit_idtr_write = 0x006a;
static constexpr std::int32_t vmexit_gdtr_write = 0x006b;
static constexpr std::int32_t vmexit_ldtr_write = 0x006c;
static constexpr std::int32_t vmexit_tr_write = 0x006d;
static constexpr std::int32_t vmexit_rdtsc = 0x006e;
static constexpr std::int32_t vmexit_rdpmc = 0x006f;
static constexpr std::int32_t vmexit_pushf = 0x0070;
static constexpr std::int32_t vmexit_popf = 0x0071;
static constexpr std::int32_t vmexit_cpuid = 0x0072;
static constexpr std::int32_t vmexit_rsm = 0x0073;
static constexpr std::int32_t vmexit_iret = 0x0074;
static constexpr std::int32_t vmexit_swint = 0x0075;
static constexpr std::int32_t vmexit_invd = 0x0076;
static constexpr std::int32_t vmexit_pause = 0x0077;
static constexpr std::int32_t vmexit_hlt = 0x0078;
static constexpr std::int32_t vmexit_invlpg = 0x0079;
static constexpr std::int32_t vmexit_invlpga = 0x007a;
static constexpr std::int32_t vmexit_ioio = 0x007b;
static constexpr std::int32_t vmexit_msr = 0x007c;
static constexpr std::int32_t vmexit_task_switch = 0x007d;
static constexpr std::int32_t vmexit_ferr_freeze = 0x007e;
static constexpr std::int32_t vmexit_shutdown = 0x007f;
static constexpr std::int32_t vmexit_vmrun = 0x0080;
static constexpr std::int32_t vmexit_vmmcall = 0x0081;
static constexpr std::int32_t vmexit_vmload = 0x0082;
static constexpr std::int32_t vmexit_vmsave = 0x0083;
static constexpr std::int32_t vmexit_stgi = 0x0084;
static constexpr std::int32_t vmexit_clgi = 0x0085;
static constexpr std::int32_t vmexit_skinit = 0x0086;
static constexpr std::int32_t vmexit_rdtscp = 0x0087;
static constexpr std::int32_t vmexit_icebp = 0x0088;
static constexpr std::int32_t vmexit_wbinvd = 0x0089;
static constexpr std::int32_t vmexit_monitor = 0x008a;
static constexpr std::int32_t vmexit_mwait = 0x008b;
static constexpr std::int32_t vmexit_mwait_conditional = 0x008c;
static constexpr std::int32_t vmexit_xsetbv = 0x008d;
static constexpr std::int32_t vmexit_efer_write_trap = 0x008f;
static constexpr std::int32_t vmexit_cr0_write_trap = 0x0090;
static constexpr std::int32_t vmexit_cr1_write_trap = 0x0091;
static constexpr std::int32_t vmexit_cr2_write_trap = 0x0092;
static constexpr std::int32_t vmexit_cr3_write_trap = 0x0093;
static constexpr std::int32_t vmexit_cr4_write_trap = 0x0094;
static constexpr std::int32_t vmexit_cr5_write_trap = 0x0095;
static constexpr std::int32_t vmexit_cr6_write_trap = 0x0096;
static constexpr std::int32_t vmexit_cr7_write_trap = 0x0097;
static constexpr std::int32_t vmexit_cr8_write_trap = 0x0098;
static constexpr std::int32_t vmexit_cr9_write_trap = 0x0099;
static constexpr std::int32_t vmexit_cr10_write_trap = 0x009a;
static constexpr std::int32_t vmexit_cr11_write_trap = 0x009b;
static constexpr std::int32_t vmexit_cr12_write_trap = 0x009c;
static constexpr std::int32_t vmexit_cr13_write_trap = 0x009d;
static constexpr std::int32_t vmexit_cr14_write_trap = 0x009e;
static constexpr std::int32_t vmexit_cr15_write_trap = 0x009f;
static constexpr std::int32_t vmexit_npf = 0x0400;
static constexpr std::int32_t avic_incomplete_ipi = 0x0401;
static constexpr std::int32_t avic_noaccel = 0x0402;
static constexpr std::int32_t vmexit_vmgexit = 0x0403;
static constexpr std::int32_t vmexit_invalid = -1;