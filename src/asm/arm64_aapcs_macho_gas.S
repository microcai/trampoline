
.align 4
_trampoline_entry_code_length:
    mov x0, #16
    ret

.align 4
_trampoline_entry_point:
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

.align 4
__asm_get_rax:
    mov x0, x10
    ret

.global _trampoline_entry_code_length
.global _trampoline_entry_point
.global __asm_get_rax
