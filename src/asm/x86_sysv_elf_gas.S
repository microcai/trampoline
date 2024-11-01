
.set TRAMPOLINE_OFFSEET, 24

.align 16
trampoline_entry:
    call get_eip
    mov %ecx, %eax
	jmp * (jmp_target-trampoline_entry) (%ecx)
get_eip:
    mov (%esp), %ecx // 3 字节
    sub $5 , %ecx // 3 字节
    ret
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
    mov %ecx, %eax
    ret

.data
trampoline_entry_code_length: .quad TRAMPOLINE_OFFSEET

.text

.global trampoline_entry_code_length
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
