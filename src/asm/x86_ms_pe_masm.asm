
.MODEL FLAT,C

.data

CODE_LEN EQU 24

trampoline_entry_code_length DWORD CODE_LEN

PUBLIC trampoline_entry_code_length

.CODE

trampoline_code:
    call get_eip
    mov ecx, eax
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

setup_trampoline proc public
    enter 0, 0
    push edi
    push esi
    mov edi, DWORD PTR [ebp + 8] ; <-- edi is now _jit_code
    lea esi, trampoline_code ; < -- esi is now &trampoline_code
    mov ecx, CODE_LEN
    rep movsb
    mov esi, DWORD PTR [ebp + 12] ; <-- esi is now call_target
    mov DWORD PTR [edi], esi
    pop esi
    pop edi
    leave
    ret
setup_trampoline endp

_asm_get_rax proc
    ret
_asm_get_rax endp

end