
.set TRAMPOLINE_OFFSEET, 24

.text
.align 16
trampoline_entry:
    mov trampoline_entry@GOTPCREL(%rip),%rax
    mov %rax, %r10
    jmp * (jump_table - trampoline_entry)(%rax)
.space TRAMPOLINE_OFFSEET - (. - trampoline_entry), 0x90
jump_table:
    .space 8, 0xcc

.align 16
.global _asm_get_rax
_asm_get_rax:
    mov %r10, %rax
    ret

.align 4
.global setup_trampoline
setup_trampoline:
    enter $0, $0
    // rdi = 参数1 _jit_code
    // rsi = 参数2 call_target
    mov %rsi, %r10
    mov trampoline_entry@GOTPCREL(%rip),%rsi // rsi is now &trampoline_entry
    mov $(TRAMPOLINE_OFFSEET), %rcx // rcx is now size of trampoline_entry
    rep movsb
    mov %r10,%rsi // <-- rsi is now call_target
    mov %rsi, (%rdi)
    leave
    ret


#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
