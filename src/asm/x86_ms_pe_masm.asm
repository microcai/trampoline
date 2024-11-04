
.MODEL FLAT,C

.CODE

ALIGN 4
clean_up_code:
    add esp, 4
    ret

ALIGN 4
trampoline_code:
    call get_eip
    push eax;
    mov ecx, eax
    push DWORD PTR [eax + (_clean_up_code_address - trampoline_code)]
    jmp  DWORD PTR [eax + (_callback_trunk_cdecl_x86_address - trampoline_code)]
get_eip:
    mov eax, DWORD PTR [esp]
    sub  eax,5
    ret
ALIGN 4
_callback_trunk_cdecl_x86_address:
    DD 0
_clean_up_code_address:
    DD clean_up_code
_trampoline_code_end:

ALIGN 4
trampoline2_code:
    call get2_eip
    mov eax, ecx
    jmp  DWORD PTR [eax + (_callback_trunk_stdcall_x86_address - trampoline2_code)]
get2_eip:
    mov ecx, DWORD PTR [esp]
    sub  ecx,5
    ret
ALIGN 4
_callback_trunk_stdcall_x86_address:
    DD 0
_trampoline2_code_end:

ALIGN 4
generate_trampoline proc public
    mov eax, DWORD PTR [ebp + 14];
    test eax, eax;
    jnz generate_trampoline2;
    push ebp
    mov  ebp, esp
    push edi
    push esi
    
    mov edi, DWORD PTR [ebp + 8] ; <-- edi is now _jit_code
    lea esi, trampoline_code ; < -- esi is now &trampoline_code
    mov ecx, (_callback_trunk_cdecl_x86_address - trampoline_code)
    rep movsb
    mov esi, DWORD PTR [ebp + 12] ; <-- esi is now call_target
    mov DWORD PTR [edi], esi
    add edi, 4
    lea esi, clean_up_code ; <-- esi is now &clean_up_code
    mov DWORD PTR [edi], esi
    pop esi
    pop edi
    mov eax, (_trampoline_code_end - trampoline_code)
    leave
    ret
generate_trampoline endp

ALIGN 4
generate_trampoline2 proc public
    push ebp
    mov  ebp, esp
    push edi
    push esi
    mov edi, DWORD PTR [ebp + 8] ; <-- edi is now _jit_code
    lea esi, trampoline2_code ; < -- esi is now &trampoline2_code
    mov ecx, (_callback_trunk_stdcall_x86_address - trampoline2_code)
    rep movsb
    mov esi, DWORD PTR [ebp + 12] ; <-- esi is now call_target
    mov DWORD PTR [edi], esi
    pop esi
    pop edi
    mov eax, (_trampoline2_code_end - trampoline2_code)
    leave
    ret
generate_trampoline2 endp

ALIGN 4
_asm_get_this_pointer proc
    ret
_asm_get_this_pointer endp

end