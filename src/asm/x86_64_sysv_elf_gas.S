
.set TRAMPOLINE_OFFSEET, 16

.text
.align 16
trampoline_entry:
    mov trampoline_entry@GOTPCREL(%rip),%rax
    jmp * (jump_table - trampoline_entry)(%rax)
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0x90
jump_table:
    .space 8, 0xcc
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
