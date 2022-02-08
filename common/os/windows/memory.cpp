#include "memory.hpp"

namespace memory {
namespace internal {


void* allocate_pages(usize size) {
    void *memory = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return memory;
}

void* allocate_pages(void* base_address, usize size) {
    void *memory = VirtualAlloc(base_address, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return memory;
}

void free_pages(void* memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}


} // namespace internal
} // namespace memory
