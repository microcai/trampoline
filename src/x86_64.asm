

.code

trampoline_entry_point proc
    lea rax, $
    nop
    jmp QWORD PTR [$+8]
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