#include "memory.hpp"
#include <sys/mman.h>

namespace memory {
namespace internal {


static void *allocations[1024] {};
static usize allocations_sizes[1024] {};
int allocations_count = 0;

void* allocate_pages(void* base_address, uint64 size) {
    void* memory = mmap(base_address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        return 0;
    }

    allocations[allocations_count] = memory;
    allocations_sizes[allocations_count] = size;
    allocations_count += 1;

    return memory;
}

void* allocate_pages(uint64 size) {
    void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        return 0;
    }

    allocations[allocations_count] = memory;
    allocations_sizes[allocations_count] = size;
    allocations_count += 1;

    return memory;
}

void free_pages(void* memory, usize size) {
    int ec = munmap(memory, size);
    ASSERT(ec == 0);
}

void free_pages(void *memory) {
    for (int i = 0; i < allocations_count; i++) {
        if (allocations[i] == memory) {
            free_pages(memory, allocations_sizes[i]);
            allocations_count -= 1;
            return;
        }
    }
}

// const char* get_allocate_pages_error() {
//     switch (errno) {
//         case EACCES: return "A file descriptor refers to a non-regular file.";
//         case EAGAIN: return "The file has been locked, or too much memory has been locked.";
//         case EBADF: return "fd is not a valid file descriptor (and MAP_ANONYMOUS was not set).";
//         case EEXIST: return "MAP_FIXED_NOREPLACE was specified in flags, and the range "
//                             "covered by addr and length clashes with an existing mapping.";
//         case EINVAL: return "We don't like addr, length, or offset (e.g., they are too large, or not aligned on a page boundary).";
//         // EINVAL (since Linux 2.6.12) length was 0.
//         // EINVAL flags contained none of MAP_PRIVATE, MAP_SHARED, or MAP_SHARED_VALIDATE.
//         case ENFILE: return "The system-wide limit on the total number of open files has been reached.";
//         case ENODEV: return "The underlying filesystem of the specified file does not support memory mapping.";
//         case ENOMEM: return "No memory is available.";
//         // ENOMEM The process's maximum number of mappings would have been
//         //        exceeded.  This error can also occur for munmap(), when
//         //        unmapping a region in the middle of an existing mapping,
//         //        since this results in two smaller mappings on either side
//         //        of the region being unmapped.
//         // ENOMEM (since Linux 4.7) The process's RLIMIT_DATA limit,
//         //        described in getrlimit(2), would have been exceeded.

//         case EOVERFLOW: return "On 32-bit architecture together with the large file "
//                                "extension (i.e., using 64-bit off_t): the number of pages "
//                                "used for length plus number of pages used for offset would "
//                                "overflow unsigned long (32 bits).";

//         case EPERM: return "The prot argument asks for PROT_EXEC but the mapped area "
//                            "belongs to a file on a filesystem that was mounted no-exec.";
//         // EPERM  The operation was prevented by a file seal; see fcntl(2).
//         // EPERM  The MAP_HUGETLB flag was specified, but the caller was not
//         //        privileged (did not have the CAP_IPC_LOCK capability) and
//         //        is not a member of the sysctl_hugetlb_shm_group group; see
//         //        the description of /proc/sys/vm/sysctl_hugetlb_shm_group
//         //        in

//         case ETXTBSY: return "MAP_DENYWRITE was set but the object specified by fd is "
//                              "open for writing.";
//         default:
//             return "Unknown error occured!\n";
//     }
// }

} // internal
} // memory
