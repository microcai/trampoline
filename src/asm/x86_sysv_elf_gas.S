
trampoline_entry_code_length:
    mov $24,%eax
    ret

trampoline_entry_point:
    call get_eip    //5 字节
    mov %eax, -16(%esp)
	jmpl * 24(%eax)
get_eip:
    mov (%esp), %eax // 3 字节
    sub $5 , %eax // 3 字节
    ret // 1 字节
    nop // 这里就是 *16(%eax) 指向的地方了 会存储 4 字节的 pointer
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
    nop
    mov -12(%ebp), %eax
    ret


.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
