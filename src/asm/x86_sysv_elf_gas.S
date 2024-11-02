
.set TRAMPOLINE_OFFSEET, 32

.text

.align 4
.global setup_trampoline
setup_trampoline:
    enter $0, $0
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
    sub $5 , %eax // 3 字节
    push %eax // this
    mov  (clean_up_ret_address_label-trampoline_entry)(%eax), %ecx
    push %ecx
    // 这里 push + jmp 是一种高级的，手动设置返回地址的 call
    // 当 [jmp_target] 完成的时候，会返回到 clean_up_ret.
    // 因为返回的时候，这段代码可能已经不在内存了，所以得返回到原始的 .text 的 clean_up_ret
    // 故而需要初始化时填入 clean_up_ret 的具体地址，而无法依靠 eip 定位
    // 否则这里其实直接使用 call jmp_target 就好了
	jmp *TRAMPOLINE_OFFSEET(%eax)
clean_up_ret:
    add $4, %esp
    ret
get_eip:
    mov (%esp), %eax // 3 字节
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
