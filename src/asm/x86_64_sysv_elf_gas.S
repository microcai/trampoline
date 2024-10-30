.p2align 16
trampoline_entry_code_length:
    mov $16,%rax
    ret

.p2align 16
trampoline_entry_point:
    lea -7(%rip), %rax
    nop
    jmp *16(%rax)
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

.p2align 16
_asm_get_rax:
    ret

.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
