.code
KTRAP_FRAME_SIZE    equ 190h
MACHINE_FRAME_SIZE  equ 28h
extern sv_handle_vm_exit : proc

push_aq macro
    push    rax
    push    rcx
    push    rdx
    push    rbx
    push    -1
    push    rbp
    push    rsi
    push    rdi
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15
    endm

pop_aq macro
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rbp
    add     rsp, 8
    pop     rbx
    pop     rdx
    pop     rcx
    pop     rax
    endm

_sldt proc PUBLIC
    sldt    WORD PTR [rcx]
    ret
_sldt endp

_str proc PUBLIC
    str     WORD PTR [rcx]
    ret
_str endp

launch_vm proc frame
    mov     rsp, rcx

sv_launch_vm_loop:
    mov     rax, [rsp]
    vmload  rax
    vmrun   rax
    vmsave  rax

    .pushframe
    sub     rsp, KTRAP_FRAME_SIZE
    .allocstack KTRAP_FRAME_SIZE - MACHINE_FRAME_SIZE + 100h
    push_aq                                 ; rsp now 0x210 below original stack top

    mov     rdx, rsp                        ; arg2: guest_registers_t*
    mov     rcx, [rsp + 220h]      ; self: 190h (KTRAP_FRAME_SIZE) + 80h (push_aq) + 10h (self offset)

    sub     rsp, 80h
    movaps  xmmword ptr [rsp + 20h], xmm0
    movaps  xmmword ptr [rsp + 30h], xmm1
    movaps  xmmword ptr [rsp + 40h], xmm2
    movaps  xmmword ptr [rsp + 50h], xmm3
    movaps  xmmword ptr [rsp + 60h], xmm4
    movaps  xmmword ptr [rsp + 70h], xmm5
    .endprolog

    call    sv_handle_vm_exit

    movaps  xmm5, xmmword ptr [rsp + 70h]
    movaps  xmm4, xmmword ptr [rsp + 60h]
    movaps  xmm3, xmmword ptr [rsp + 50h]
    movaps  xmm2, xmmword ptr [rsp + 40h]
    movaps  xmm1, xmmword ptr [rsp + 30h]
    movaps  xmm0, xmmword ptr [rsp + 20h]
    add     rsp, 80h

    test    al, al
    pop_aq

    jnz     sv_launch_vm_done
    add     rsp, KTRAP_FRAME_SIZE
    jmp     sv_launch_vm_loop

sv_launch_vm_done:
    mov     rsp, rcx
    mov     ecx, 'MVSS'
    jmp     rbx

launch_vm endp
end