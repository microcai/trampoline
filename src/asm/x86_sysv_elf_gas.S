
.set TRAMPOLINE_OFFSEET, 24

.align 16
trampoline_entry:
    call get_eip
    pop %ecx // this is the return address
    push %eax // this is the this address, pushed as first argument
    push %ecx // this is the return address pushed again
	jmp * (jmp_target-trampoline_entry) (%eax)
get_eip:
    mov (%esp), %eax // 3 字节
    sub $5 , %eax // 3 字节
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
