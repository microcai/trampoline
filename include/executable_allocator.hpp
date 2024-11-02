
#pragma once

#include <cstddef>

struct ExecutableAllocator
{
	void* allocate(std::size_t size);

	void deallocate(void* raw_ptr, std::size_t size);
	void protect(void* raw_ptr, std::size_t size);
	void unprotect(void* raw_ptr, std::size_t size);
};

