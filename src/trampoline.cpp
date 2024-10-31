
#include "trampoline.hpp"

extern "C" void trampoline_entry_point();

const unsigned char* trampoline::_machine_code_template()
{
	#if defined (__aarch64__)
	static constinit uint32_t arm64_trampoline_code[] = {
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
