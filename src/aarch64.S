
trampoline_entry_code_length:
    mov x0, 16
    ret

trampoline_entry_point:
    lea -7(%rip), %rax
    nop
    jmp *2(%rip)
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
