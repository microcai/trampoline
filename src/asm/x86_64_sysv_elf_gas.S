
.set TRAMPOLINE_OFFSEET, 16

.text
.align 16
trampoline_entry:
    lea -7(%rip), %rax
    mov %rax, -16(%rsp)
    jmp *TRAMPOLINE_OFFSEET(%rax)
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0x90
.quad 0xcc
    ret

.align 4
.text
trampoline_entry_code_length: .quad TRAMPOLINE_OFFSEET
.global trampoline_entry_code_length

.align 16
.global _asm_get_rax
_asm_get_rax:
    ret

.align 4
.global trampoline_entry_point
trampoline_entry_point:
    mov trampoline_entry@GOTPCREL(%rip),%rax
    ret

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
