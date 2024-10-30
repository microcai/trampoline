
trampoline_entry_code_length:
    mov x0, 16
    ret

trampoline_entry_point:
    str x30, [sp, #8]
    blr x8
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
    nop
    nop
    nop

_asm_get_rax:
    ret

.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
