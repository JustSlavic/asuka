#include "memory.hpp"

#if ASUKA_OS_WINDOWS
#include "windows/memory.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/memory.hpp"
#endif


//
// Cross-platform code.
//


namespace memory {


void* allocate_pages(uint64 size) {
    return internal::allocate_pages(size);
}

void* allocate_pages(void* base_address, uint64 size) {
    return internal::allocate_pages(base_address, size);
}

void free_pages(void* memory) {
    return internal::free_pages(memory);
}

void set(void *memory, uint8 value, usize size) {
    uint8 *m = (uint8 *)memory;
    for (usize i = 0; i < size; i++) {
        m[i] = value;
    }
}

void copy(void *destination, void *source, usize size) {
    uint8 *d = (uint8 *)destination;
    uint8 *s = (uint8 *)source;
    for (usize i = 0; i < size; i++) {
        d[i] = s[i];
    }
}


} // namespace memory
