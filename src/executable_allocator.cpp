
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "executable_allocator.hpp"

#ifdef _WIN32

void * ExecutableAllocator::allocate(std::size_t size)
{
    auto allocated_mem = VirtualAlloc(0, size, MEM_COMMIT| MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    return allocated_mem;
}

void ExecutableAllocator::deallocate(void* raw_ptr, std::size_t size)
{
	VirtualFree(raw_ptr, size, 0);
}

#else

void * ExecutableAllocator::allocate(std::size_t size)
{
  // return malloc(size);
    return mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void ExecutableAllocator::deallocate(void* raw_ptr, std::size_t size)
{
    // free(raw_ptr);
    munmap(raw_ptr, size);
}
#endif