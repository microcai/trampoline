

.code

trampoline_entry_code_length proc public
    mov rax, 16
    ret
trampoline_entry_code_length endp

trampoline_entry_point proc public
    lea rax, $
    jmp QWORD PTR [rax+16]
    nop
    nop
    nop
    nop
    nop
    nop
trampoline_entry_point endp

_asm_get_rax proc
    ret
_asm_get_rax endp

end