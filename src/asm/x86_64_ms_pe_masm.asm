

.code

ALIGN 16
trampoline_code:
    lea r10, $
    jmp QWORD PTR [real_target]
ALIGN 16
real_target:
    DQ 0

ALIGN 16
generate_trampoline proc public
    push rbp
    mov rbp, rsp
    ; rcx = 参数1 _jit_code
    ; rdx = 参数2 call_target
    push rdi
    push rsi

    mov rdi, rcx ; rdi is now _jit_code
    lea rsi, trampoline_code ; rsi is now &trampoline_entry
    mov rcx, real_target - trampoline_code
    rep movsb
    mov [rdi], rdx
    pop rsi
    pop rdi
    mov rax, real_target - trampoline_code + 8
    leave
    ret
generate_trampoline endp


ALIGN 16
_asm_get_this_pointer proc
    mov rax, r10
    ret
_asm_get_this_pointer endp

end