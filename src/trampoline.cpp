
#include "trampoline.hpp"

extern "C" // from assembly code
{
	extern void* trampoline_entry_point();
	extern long trampoline_entry_code_length;
}

void trampoline::dynamic_function_base_trampoline::setup_trampoline(void* wrap_func_ptr)
{
	auto code_len = trampoline_entry_code_length;
	auto machine_code_template = trampoline_entry_point();

	memcpy(_jit_code, machine_code_template, code_len);
	memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
	ExecutableAllocator{}.protect(this, sizeof (*this));
}
