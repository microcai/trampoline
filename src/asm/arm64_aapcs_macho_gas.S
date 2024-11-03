
.set TRAMPOLINE_OFFSEET, 16

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
.global __asm_get_this_pointer
__asm_get_this_pointer:
    mov x0, x10
    ret

.align 4
.global _setup_trampoline
_setup_trampoline:
    stp x29, x30, [sp, -32]!
    mov x29, sp
    stp x19, x20, [sp, 16]
    mov x19, x0
    mov x20, x1
    mov x2, TRAMPOLINE_OFFSEET
    adr x1, trampoline_entry
    bl _memcpy
    str x20, [x19, TRAMPOLINE_OFFSEET ]
    ldp x19, x20, [sp, 16]
    ldp x29, x30, [sp], 32
    ret
