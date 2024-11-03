
#include "trampoline.hpp"

extern "C" // from assembly code
{
	extern std::size_t generate_trampoline(void* jit_code_address, const void* call_target);
}

void trampoline::dynamic_function_base::generate_trampoline(const void* wrap_func_ptr)
{
	auto code_size = ::generate_trampoline(_jit_code, wrap_func_ptr);
	if (code_size > _jit_code_size)
	{
		m_current_alloca = (code_size /64 +1) * 64;
	}
	ExecutableAllocator{}.protect(this, sizeof (*this));
}

trampoline::dynamic_function_base::once_allocator::once_allocator()
{}

trampoline::dynamic_function_base::once_allocator::once_allocator(dynamic_function_base* parent)
	: _parent(parent)
{}

void* trampoline::dynamic_function_base::once_allocator::allocate(std::size_t size)
{
	return _parent->allocate_from_jit_code(size);
}

void trampoline::dynamic_function_base::once_allocator::deallocate(void* ptr, int s)
{
	// do nothing !
}

trampoline::dynamic_function_base::once_allocator trampoline::dynamic_function_base::get_allocator()
{
	return trampoline::dynamic_function_base::once_allocator(this);
}
