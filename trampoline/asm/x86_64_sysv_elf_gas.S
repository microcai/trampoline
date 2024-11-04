
.text
.align 16
trampoline_callback_entry:
    # 获取本代码的起始位置，存入 r10 寄存器
    mov trampoline_callback_entry@GOTPCREL(%rip),%r10
    // 获取 目标跳转地址，存入 rax 寄存器
    mov (jump_address - trampoline_callback_entry)(%r10), %rax
    // 跳入 rax 寄存器所指示的地址
    jmp * %rax

.align 16
jump_address: // 保留 8 个字节的空间，generate_trampoline 会在此处填入跳转地址
    .space 8, 0xcc
.align 16

// 从 trampoline_entry 到这里，就是一个模板。这段 位置无关代码
// 会被拷贝到动态创建的内存里，并且在 jump_address 所在的位置填入
// 跳转地址。这个跳转的目标还不是用户的 lambda, 而是
// static R dynamic_function::_callback_trunk_cdecl(Args...);
// 在这里，它从 r10 寄存器获取到 this 指针, 然后 跳入
// R dynamic_function::operator()(Args...)
// 因为有了 this。然后 operator() 再去调用它用 function<> 存储
// 的真正的用户的 lambda
// 由于 _callback_trunk_cdecl 的签名，和用设定的回调是一样的，所以
// 直接使用 jmp 去调用 _callback_trunk_cdecl，它返回的时候，就会直接
// 返回掉调用的地方，而不会回到这里。毕竟等 _callback_trunk_cdecl 执行
// 完毕的时候，这段“动态分配”出来的代码，可能已经被“回收”了。
end_of_trampoline_callback_entry:


.global _asm_get_this_pointer
_asm_get_this_pointer:
    // _callback_trunk_cdecl(Args...) 会调用此 api
    // 获取 r10 寄存器的值，从而获取 _jit_code 的地址
    // 这个地址，就是 dynamic_function 的 this
    // 有了 this，就有了一切
    mov %r10, %rax
    ret

.align 16

// 本函数签名 extern "C" int generate_trampoline(void* _jit_code, void (*call_target)() )
.global generate_trampoline
generate_trampoline:
    // rdi = 参数1 _jit_code
    // rsi = 参数2 call_target
    push %rbp
    mov %rsp, %rbp

    // 此4行汇编的本质是 memcpy(_jit_code, &trampoline_callback_entry, jump_address offset)
    mov %rsi, %r10
    mov trampoline_callback_entry@GOTPCREL(%rip),%rsi // rsi is now &trampoline_callback_entry
    mov $(jump_address - trampoline_callback_entry), %rcx // rcx is now size
    rep movsb

    // 接下来，是往 jump_address offset 处存入 call_target
    // rdi 并不指向 _jit_code, 而是被 rep movsb 更新到 _jit_code + jump_address offset
    mov %r10, (%rdi)
    // 返回模板代码的大小
    mov $(end_of_trampoline_callback_entry-trampoline_callback_entry), %rax
    leave
    ret


#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
