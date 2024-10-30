
#include "trampoline.hpp"

extern "C" void trampoline_entry_point();

const unsigned char* trampoline::_machine_code_template()
{
	auto address = &trampoline_entry_point;
	return (const unsigned char*) address;
}
