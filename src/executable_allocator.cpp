
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "executable_allocator.hpp"

#ifdef _WIN32

void * ExecutableAllocator::allocate(std::size_t size)
{
    auto allocated_mem = VirtualAlloc(0, size, MEM_COMMIT| MEM_RESERVE, PAGE_READWRITE);
    return allocated_mem;
}

void ExecutableAllocator::deallocate(void* raw_ptr, std::size_t size)
{
	VirtualFree(raw_ptr, size, 0);
}

void ExecutableAllocator::protect(void* raw_ptr, std::size_t size)
{
    DWORD old;
    VirtualProtect(raw_ptr, size, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), raw_ptr, size);
}

void ExecutableAllocator::unprotect(void* raw_ptr, std::size_t size)
{
    DWORD old;
    VirtualProtect(raw_ptr, size, PAGE_READWRITE, &old);
}

#else

#include <iostream>

void * ExecutableAllocator::allocate(std::size_t size)
{
  // return malloc(size);
#ifdef MAP_JIT
    auto out = mmap(0, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);
#else
    auto out = mmap(0, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    if (out == nullptr)
    {
        std::cerr << "unable to alloc executable page\n";
        std::terminate();
    }
    return out;
}

void ExecutableAllocator::deallocate(void* raw_ptr, std::size_t size)
{
    // free(raw_ptr);
    munmap(raw_ptr, size);
}

void ExecutableAllocator::protect(void* raw_ptr, std::size_t size)
{
    mprotect(raw_ptr, size, PROT_EXEC|PROT_READ);
}

void ExecutableAllocator::unprotect(void* raw_ptr, std::size_t size)
{
    mprotect(raw_ptr, size, PROT_READ|PROT_WRITE);
}

#endif
