
.MODEL FLAT
.CODE

trampoline_entry_code_length proc public
    mov eax, 16
    ret
trampoline_entry_code_length endp

trampoline_entry_point proc public
    call get_eip
    sub  eax,5
    jmp  DWORD PTR[eax + 16]
get_eip:
    mov eax, DWORD PTR [esp]
    ret
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