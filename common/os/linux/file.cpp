#include <defines.hpp>

#include "file.hpp"
#include <os/memory.hpp>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


namespace os {
namespace internal {

byte_array load_entire_file(const char* filename)
{
    byte_array result = {};

    struct stat st;
    stat(filename, &st);
    usize size = st.st_size;

    void *memory = memory::allocate_pages(size + 1);
    if (memory)
    {
        int fd = open(filename, O_RDONLY);
        if (fd > 0)
        {
            usize bytes_read = read(fd, memory, size);
            if (bytes_read < size) {
                // @warning: Read less than expected!
            }

            *((uint8 *)memory + bytes_read) = 0; // Make sure string is null-terminated

            result.data = (memory::byte *)memory;
            result.size = size;

            close(fd);
        }
        else
        {
            memory::free_pages(memory);
        }
    }

    return result;
}


bool write_file(const char* filename, byte_array file) {
    NOT_IMPLEMENTED();
    return false;
}


} // namespace internal
} // namespace os
