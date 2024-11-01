
.set TRAMPOLINE_OFFSEET, 16

.align 16
trampoline_entry:
    mov %eax, -24(%esp)
	jmp * (jmp_target-trampoline_entry) (%eax)
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0x90
jmp_target:
.space 4, 0xcc
    nop
    ret

.align 4
.global trampoline_entry_point
trampoline_entry_point:
    lea trampoline_entry@GOTOFF(%ebx),%eax
    ret

_asm_get_rax:
    nop
    mov -20(%ebp), %eax
    ret

.data
trampoline_entry_code_length: .quad TRAMPOLINE_OFFSEET

.text

.global trampoline_entry_code_length
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
