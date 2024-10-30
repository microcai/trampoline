
.MODEL FLAT
.CODE

_trampoline_entry_code_length proc public
    mov eax, 16
    ret
_trampoline_entry_code_length endp

_trampoline_entry_point proc public
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
_trampoline_entry_point endp

__asm_get_rax proc
    ret
__asm_get_rax endp

end