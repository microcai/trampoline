
.text

.align 4
.global generate_trampoline
generate_trampoline:
    push %ebp
    mov %esp, %ebp
    // ebp+8 = 参数1
    // ebp+12 = 参数2
    push %edi
    push %esi
    mov 8(%ebp),%edi // <-- edi is now _jit_code
    lea trampoline_entry@GOTOFF(%ebx),%esi // esi is now &trampoline_entry
    mov $(jmp_target - trampoline_entry), %ecx // ecx is now size of trampoline_entry
    rep movsb
    mov 12(%ebp),%esi // <-- esi is now call_target
    mov %esi, (%edi)
    mov $(jmp_target - trampoline_entry), %eax
    leave
    ret

.align 8
trampoline_entry:
    call get_eip
    sub $5 , %eax // 3 字节
    jmp * (jmp_target - trampoline_entry)(%eax)
get_eip:
    mov (%esp), %eax // 3 字节
    ret
.align 8
jmp_target:
.space 4, 0xcc

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
