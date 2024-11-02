
#include "trampoline.hpp"

extern "C" // from assembly code
{
	extern void* setup_trampoline(void* jit_code_address, const void* call_target);
}

void trampoline::dynamic_function_base_trampoline::setup_trampoline(const void* wrap_func_ptr)
{
	::setup_trampoline(_jit_code, wrap_func_ptr);
	ExecutableAllocator{}.protect(this, sizeof (*this));
}
