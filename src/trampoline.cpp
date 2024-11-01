
#include "trampoline.hpp"

extern "C" void trampoline_entry_point();

const unsigned char* _machine_code_template()
{
	#if defined (__aarch64__)
	static const uint32_t arm64_trampoline_code[] = {
		0x1000000a,
		0x58000069,
		0xd61f0120,
		0xd503201f
	};

	return (const unsigned char*) arm64_trampoline_code;

	#else
	auto address = &trampoline_entry_point;
	return (const unsigned char*) address;
	#endif
}

extern "C" long trampoline_entry_code_length;

void trampoline::dynamic_function_base_trampoline::setup_trampoline(void* wrap_func_ptr)
{
	auto code_len = trampoline_entry_code_length;

	memcpy(_jit_code, _machine_code_template(), code_len);
	memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
	ExecutableAllocator{}.protect(this, sizeof (*this));
}
