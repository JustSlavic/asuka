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

namespace Asuka {
namespace memory {


void* allocate_pages(u64 size) {
    return internal::allocate_pages(size);
}

void* allocate_pages(void* base_address, u64 size) {
    return internal::allocate_pages(base_address, size);
}

void free_pages(void* memory) {
    return internal::free_pages(memory);
}

void set(void *memory, u8 value, usize size) {
    u8 *m = (u8 *)memory;
    for (usize i = 0; i < size; i++) {
        m[i] = value;
    }
}

void copy(void *destination, void *source, usize size) {
    u8 *d = (u8 *)destination;
    u8 *s = (u8 *)source;
    for (usize i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

template <typename T>
void fill_buffer(T *buffer, T value, usize size) {
    for (usize i = 0; i < size; i++) {
        buffer[i] = value;
    }
}


} // namespace memory
} // namespace Asuka
