
#include <assert.h>
#include "trampoline.hpp"

extern "C" // from assembly code
{
	extern std::size_t generate_trampoline(void* jit_code_address, const void* call_target);
}

void trampoline::dynamic_function_base::generate_trampoline(const void* wrap_func_ptr)
{
	auto code_size = ::generate_trampoline(_jit_code, wrap_func_ptr);
	assert(code_size <= _jit_code_size);
	ExecutableAllocator{}.protect(this, sizeof (*this));
}
