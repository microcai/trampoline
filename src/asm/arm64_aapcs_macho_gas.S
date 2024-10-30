
.p2align 16
trampoline_entry_code_length:
    mov x0, #16
    ret

.p2align 16
trampoline_entry_point:
    ADR x10, .
    ldr x9, dummy_ptr
    br x9
    nop
dummy_ptr: .dword 0x00000000
    .dword 0x00000000
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

.p2align 16
_asm_get_rax:
    mov x0, x10
    ret

.global trampoline_entry_code_length
.global trampoline_entry_point
.global _asm_get_rax
