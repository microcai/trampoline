
.MODEL FLAT,C

.data

CODE_LEN EQU 16

trampoline_entry_code_length DWORD CODE_LEN

PUBLIC trampoline_entry_code_length

.CODE

trampoline_code proc
    call get_eip
    jmp  DWORD PTR[eax + CODE_LEN]
get_eip:
    mov eax, DWORD PTR [esp]
    sub  eax,5
    ret
    nop
    nop
    nop
    nop
    nop
trampoline_code endp

trampoline_entry_point proc public
    lea eax, trampoline_code
    ret
trampoline_entry_point endp

_asm_get_rax proc
    ret
_asm_get_rax endp

end