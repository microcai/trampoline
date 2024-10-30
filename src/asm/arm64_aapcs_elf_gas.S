
.p2align 8
trampoline_entry_code_length:
    mov x0, #24
    ret

.p2align 8
trampoline_entry_point:
    ADR x10, .
    add x9, x10, #24
    ldr x9,[x9]
    br x9
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

.p2align 8
_asm_get_rax:
    mov x0, x10
    ret

.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
