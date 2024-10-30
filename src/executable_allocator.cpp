
#include <sys/mman.h>

#include "executable_allocator.hpp"

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