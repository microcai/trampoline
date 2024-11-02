
.set TRAMPOLINE_OFFSEET, 32

.text
.align 16
clean_up_ret:
    add $4, %esp
    ret

.align 4
.global setup_trampoline
setup_trampoline:
    enter $4, $0
    // ebp+8 = 参数1
    // ebp+12 = 参数2
    push %edi
    push %esi
    mov 8(%ebp),%edi // <-- edi is now _jit_code
    lea trampoline_entry@GOTOFF(%ebx),%esi // esi is now &trampoline_entry
    mov $(TRAMPOLINE_OFFSEET), %ecx // ecx is now size of trampoline_entry
    rep movsb
    mov 12(%ebp),%esi // <-- esi is now call_target
    movl %esi, (%edi)
    add $4, %edi
    lea clean_up_ret@GOTOFF(%ebx),%esi // esi is now &trampoline_entry
    movl %esi, (%edi)
    pop %esi
    pop %edi
    leave
    ret

.align 16
trampoline_entry:
    call get_eip
    push %eax // this
    mov  (clean_up_ret_address_label-trampoline_entry)(%eax), %ecx
    push %ecx
	jmp *TRAMPOLINE_OFFSEET(%eax)
    nop
    nop
    nop
get_eip:
    mov (%esp), %eax // 3 字节
    sub $5 , %eax // 3 字节
    ret
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0x90
jmp_target:
.space 4, 0xcc
clean_up_ret_address_label:
.space 4, 0xcc
    nop
    ret

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
