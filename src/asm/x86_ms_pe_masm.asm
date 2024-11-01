
.MODEL FLAT,C

.data

CODE_LEN EQU 16

trampoline_entry_code_length DWORD CODE_LEN

PUBLIC trampoline_entry_code_length

.CODE

_trampoline_entry_code_length proc public
    mov eax, 16
    ret
_trampoline_entry_code_length endp

_trampoline_entry_point proc public
    call get_eip
    jmp  DWORD PTR[eax + 16]
get_eip:
    mov eax, DWORD PTR [esp]
    sub  eax,5
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