

.data

CODE_LEN EQU 16

trampoline_entry_code_length QWORD CODE_LEN

PUBLIC trampoline_entry_code_length

.code

; trampoline_entry_code_length proc public
;     mov rax, 16
;     ret
; trampoline_entry_code_length endp

trampoline_entry_point proc public
    lea rax, $
    jmp QWORD PTR [rax+ CODE_LEN]
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