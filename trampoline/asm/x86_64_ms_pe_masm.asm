

CODE_LEN EQU 16

.code

trampoline_code:
    lea rax, $
    mov r10, rax
    jmp QWORD PTR [rax+ CODE_LEN]
    nop
    nop
    nop
    nop
    nop
    nop

generate_trampoline proc public
    enter 0, 0
    ; rcx = 参数1 _jit_code
    ; rdx = 参数2 call_target
    push rdi
    push rsi

    mov rdi, rcx ; rdi is now _jit_code
    lea rsi, trampoline_code ; rsi is now &trampoline_entry
    mov rcx, CODE_LEN
    rep movsb
    mov [rdi], rdx
    pop rsi
    pop rdi
    mov rax, CODE_LEN + 8
    leave
    ret
generate_trampoline endp


_asm_get_this_pointer proc
    mov rax, r10
    ret
_asm_get_this_pointer endp

end