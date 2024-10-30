
#include "trampoline.hpp"

extern "C" void trampoline_entry_point();

const unsigned char* trampoline::_machine_code_template()
{
#if defined(__x86_64__) ||defined(_M_AMD64)
	static constinit unsigned char machine_code_template[] = {
		0x90, 0x48, 0x8d, 0x05, 0xf8, 0xff, 0xff, 0xff, // nop; lea rax, [rip-8]
		0xff, 0x25, 0x02, 0x00, 0x00, 0x00, 0x90, 0x90, // jmp [rip+2]; nop; nop
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // c_function_ptr::do_invoke() address
	};
	return machine_code_template;
#else
	auto address = &trampoline_entry_point;

	return (const unsigned char*) address;
#endif
}
