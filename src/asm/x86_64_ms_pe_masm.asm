

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

setup_trampoline proc public
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
    leave
    ret
setup_trampoline endp


_asm_get_rax proc
    mov rax, r10
    ret
_asm_get_rax endp

end