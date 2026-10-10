#pragma once
#include <cstdint>

namespace guest {
    namespace exit {

        void inject_gp( virtual_processor_data_t* vp_data ) {
            std::uint64_t event = 0;
            event |= 13;
            event |= ( 3ull << 8 );
            event |= ( 1ull << 11 );
            event |= ( 1ull << 31 );
            vp_data->m_guest_vmcb.m_control_area.m_event_inj = event;
        }

        void advance_rip( virtual_processor_data_t* vp_data ) {
            const auto next_rip = vp_data->m_guest_vmcb.m_control_area.m_n_rip;
            const auto current_rip = vp_data->m_guest_vmcb.m_state_save_area.m_rip;

            if ( next_rip != 0 ) {
                vp_data->m_guest_vmcb.m_state_save_area.m_rip = next_rip;
                return;
            }

            const auto num_bytes =
                vp_data->m_guest_vmcb.m_control_area.m_num_of_bytes_fetched;
            if ( num_bytes == 0 ) {
                inject_gp( vp_data );
                return;
            }

            vp_data->m_guest_vmcb.m_state_save_area.m_rip = current_rip + 2;
        }

        void handle_cpuid( virtual_processor_data_t* vp_data, guest_context_t* ctx ) {
            int regs[ 4 ] = { 0 };

            const auto leaf =
                static_cast< int >( vp_data->m_guest_vmcb.m_state_save_area.m_rax );
            const auto sub_leaf =
                static_cast< int >( ctx->m_vp_regs->m_rcx );

            __cpuidex( regs, leaf, sub_leaf );

            switch ( leaf ) {
                case cpuid_processor_features:
                    regs[ 2 ] |= cpuid_ecx_hypervisor_present;
                    break;

                case cpuid_hv_vendor:
                    regs[ 0 ] = 0x40000001;
                    regs[ 1 ] = 'tcoN';
                    regs[ 2 ] = 'anru';
                    regs[ 3 ] = '   l';
                    break;

                case cpuid_hv_interface:
                    regs[ 0 ] = 0x30237648;
                    regs[ 1 ] = regs[ 2 ] = regs[ 3 ] = 0;
                    break;

                case cpuid_unload:
                    if ( sub_leaf == cpuid_unload ) {
                        segment_attribute_t attrib;
                        attrib.m_as_uint16 =
                            vp_data->m_guest_vmcb.m_state_save_area.m_ss_attrib;
                        if ( attrib.m_dpl == dpl_system )
                            ctx->m_exit_vm = true;
                    }
                    break;

                default:
                    break;
            }

            vp_data->m_guest_vmcb.m_state_save_area.m_rax = regs[ 0 ];
            ctx->m_vp_regs->m_rbx = regs[ 1 ];
            ctx->m_vp_regs->m_rcx = regs[ 2 ];
            ctx->m_vp_regs->m_rdx = regs[ 3 ];

            advance_rip( vp_data );
        }

        void handle_msr( virtual_processor_data_t* vp_data, guest_context_t* ctx ) {
            const auto msr =
                static_cast< std::uint32_t >( ctx->m_vp_regs->m_rcx );
            const bool write_access =
                vp_data->m_guest_vmcb.m_control_area.m_exit_info1 != 0;

            if ( msr != ia32_msr_efer ) {
                KeBugCheckEx( 0xDEAD0004, msr, write_access,
                    vp_data->m_guest_vmcb.m_state_save_area.m_rip, 0 );
            }

            if ( !write_access ) {
                const std::uint64_t value =
                    vp_data->m_guest_vmcb.m_state_save_area.m_efer;
                ctx->m_vp_regs->m_rax = value & 0xffffffff;
                ctx->m_vp_regs->m_rdx = value >> 32;
            }
            else {
                const std::uint64_t value =
                    ( ctx->m_vp_regs->m_rdx << 32 )
                    | ( ctx->m_vp_regs->m_rax & 0xffffffff );
                if ( !( value & efer_svme ) ) {
                    inject_gp( vp_data );
                    return;
                }
                vp_data->m_guest_vmcb.m_state_save_area.m_efer = value;
            }

            advance_rip( vp_data );
        }

        void handle_vmrun( virtual_processor_data_t* vp_data, guest_context_t* ) {
            inject_gp( vp_data );
        }

        void handle_exception( virtual_processor_data_t* vp_data, guest_context_t* ) {
            const auto exit_code =
                vp_data->m_guest_vmcb.m_control_area.m_exit_code;
            const std::uint64_t vector = exit_code - vmexit_exception_de;

            std::uint64_t event = 0;
            event |= vector;
            event |= ( 3ull << 8 );
            event |= ( 1ull << 31 );

            switch ( vector ) {
                case 8: case 10: case 11: case 12:
                case 13: case 14: case 17:
                    event |= ( 1ull << 11 );
                    event |= ( static_cast< std::uint64_t >(
                        static_cast< std::uint32_t >(
                            vp_data->m_guest_vmcb.m_control_area.m_exit_info1 ) ) << 32 );
                    break;
                default:
                    break;
            }

            vp_data->m_guest_vmcb.m_control_area.m_event_inj = event;
        }

        void handle_xsetbv( virtual_processor_data_t* vp_data, guest_context_t* ctx ) {
            const std::uint32_t xcr = static_cast< std::uint32_t >( ctx->m_vp_regs->m_rcx );
            const std::uint64_t value =
                ( ctx->m_vp_regs->m_rdx << 32 ) |
                ( ctx->m_vp_regs->m_rax & 0xffffffff );

            if ( !( vp_data->m_guest_vmcb.m_state_save_area.m_cr4 & 0x40000 ) ) {
                inject_gp( vp_data );
                return;
            }

            if ( xcr != 0 ) {
                inject_gp( vp_data );
                return;
            }

            _xsetbv( xcr, value );
            advance_rip( vp_data );
        }

        extern "C" bool sv_handle_vm_exit(
            virtual_processor_data_t* vp_data,
            guest_registers_t* guest_regs ) {
            __svm_vmload( vp_data->m_host_stack_layout.host_vmcb_pa );

            NT_ASSERT( vp_data->m_host_stack_layout.reserved1 == MAXUINT64 );

            guest_regs->m_rax = vp_data->m_guest_vmcb.m_state_save_area.m_rax;

            vp_data->m_host_stack_layout.trap_frame.Rsp =
                vp_data->m_guest_vmcb.m_state_save_area.m_rsp;
            vp_data->m_host_stack_layout.trap_frame.Rip =
                vp_data->m_guest_vmcb.m_control_area.m_n_rip;

            guest_context_t ctx{};
            ctx.m_vp_regs = guest_regs;
            ctx.m_exit_vm = false;

            const auto exit_code =
                vp_data->m_guest_vmcb.m_control_area.m_exit_code;

            kernel::dbg_print( "[exit] code=0x%llx rip=0x%llx\n",
                exit_code,
                vp_data->m_guest_vmcb.m_state_save_area.m_rip );

            switch ( exit_code ) {
                case vmexit_invalid:
                    KeBugCheckEx( 0xDEAD0001,
                        exit_code,
                        vp_data->m_guest_vmcb.m_state_save_area.m_rip,
                        vp_data->m_guest_vmcb.m_state_save_area.m_cr0,
                        vp_data->m_guest_vmcb.m_state_save_area.m_efer );
                    break;

                case vmexit_cpuid:
                    handle_cpuid( vp_data, &ctx );
                    break;

                case vmexit_msr:
                    handle_msr( vp_data, &ctx );
                    break;

                case vmexit_vmrun:
                    handle_vmrun( vp_data, &ctx );
                    break;

                case vmexit_xsetbv:
                    handle_xsetbv( vp_data, &ctx );
                    break;

                case vmexit_npf:
                    KeBugCheckEx( 0xDEAD0002,
                        vp_data->m_guest_vmcb.m_control_area.m_exit_info2,
                        vp_data->m_guest_vmcb.m_control_area.m_exit_info1,
                        vp_data->m_guest_vmcb.m_state_save_area.m_rip,
                        vp_data->m_guest_vmcb.m_state_save_area.m_cr3 );
                    break;
                case vmexit_shutdown:
                    KeBugCheckEx( 0xDEAD0007,
                        vp_data->m_guest_vmcb.m_state_save_area.m_rip,
                        vp_data->m_guest_vmcb.m_state_save_area.m_cr0,
                        vp_data->m_guest_vmcb.m_state_save_area.m_efer,
                        vp_data->m_guest_vmcb.m_control_area.m_exit_info1 );
                    break;
                default:
                    if ( exit_code >= vmexit_exception_de
                        && exit_code <= vmexit_exception_xf ) {
                        handle_exception( vp_data, &ctx );
                    }
                    else {
                        kernel::dbg_print( "[exit] unhandled=0x%llx rip=0x%llx info1=0x%llx info2=0x%llx\n",
                            exit_code,
                            vp_data->m_guest_vmcb.m_state_save_area.m_rip,
                            vp_data->m_guest_vmcb.m_control_area.m_exit_info1,
                            vp_data->m_guest_vmcb.m_control_area.m_exit_info2 );
                        __fastfail( 0 );
                    }
                    break;
            }

            if ( ctx.m_exit_vm ) {
                guest_regs->m_rax =
                    reinterpret_cast< std::uint64_t >( vp_data ) & MAXUINT32;
                guest_regs->m_rbx =
                    vp_data->m_guest_vmcb.m_control_area.m_n_rip;
                guest_regs->m_rcx =
                    vp_data->m_guest_vmcb.m_state_save_area.m_rsp;
                guest_regs->m_rdx =
                    reinterpret_cast< std::uint64_t >( vp_data ) >> 32;

                __svm_vmload(
                    kernel::mm_get_physical_address(
                        &vp_data->m_guest_vmcb ).m_quad_part );
                _disable( );
                __svm_stgi( );
                __writemsr( ia32_msr_efer,
                    __readmsr( ia32_msr_efer ) & ~static_cast< std::uint64_t >( efer_svme ) );
                __writeeflags( vp_data->m_guest_vmcb.m_state_save_area.m_rflags );
            }
            else {
                vp_data->m_guest_vmcb.m_state_save_area.m_rax = guest_regs->m_rax;
            }

            return ctx.m_exit_vm;
        }

    } // namespace exit
} // namespace guest