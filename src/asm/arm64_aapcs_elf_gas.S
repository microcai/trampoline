
.align 4
trampoline_entry_code_length:
    mov x0, #16
    ret

.align 4
trampoline_entry_point:
    ADR x10, .
    ldr x9, dummy_ptr
    br x9
    nop
dummy_ptr: .dword 0x00000000
    .dword 0x00000000
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

.align 4
_asm_get_rax:
    mov x0, x10
    ret

.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
