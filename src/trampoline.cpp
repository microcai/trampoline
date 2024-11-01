
#include "trampoline.hpp"

extern "C" // from assembly code
{
#if defined (__i386__) || defined(_M_IX86) || defined (__x86_64__)
	extern void* setup_trampoline(void* jit_code_address, const void* call_target);
#else
	extern void* trampoline_entry_point();
	extern long trampoline_entry_code_length;
#endif
}

void trampoline::dynamic_function_base_trampoline::setup_trampoline(const void* wrap_func_ptr)
{
#if defined (__i386__) || defined(_M_IX86) || defined (__x86_64__)
	::setup_trampoline(_jit_code, wrap_func_ptr);
#else
	auto code_len = trampoline_entry_code_length;
	auto machine_code_template = trampoline_entry_point();
	memcpy(_jit_code, machine_code_template, code_len);
	memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
#endif

	ExecutableAllocator{}.protect(this, sizeof (*this));
}
