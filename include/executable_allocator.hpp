
#pragma once

#include <cstdint>

struct ExecutableAllocator
{
	void* allocate(std::size_t size);

	void deallocate(void* raw_ptr, std::size_t size);
};

