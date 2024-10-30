
trampoline_entry_code_length:
    mov x0, 24
    ret

trampoline_entry_point:
    ADR x8, .
    add x9, x8, #20
    ldr x9,[x9]
    blr x9
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
