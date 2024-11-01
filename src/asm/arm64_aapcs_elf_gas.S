
.set TRAMPOLINE_OFFSEET, 16

.data
.align 4
.global trampoline_entry_code_length
trampoline_entry_code_length: .quad TRAMPOLINE_OFFSEET

.text

.align 4
trampoline_entry:
    ADR x10, .
    ldr x9, dummy_ptr
    br x9
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0

dummy_ptr: .dword 0xcccccccc
    .dword 0x00000000
    ret

.align 4
.global _asm_get_rax
_asm_get_rax:
    mov x0, x10
    ret

.align 4
.global trampoline_entry_point
trampoline_entry_point:
    ADR x0, trampoline_entry
    ret

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
