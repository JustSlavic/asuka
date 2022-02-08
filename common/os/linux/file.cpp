#include "file.hpp"
#include <os/memory.hpp>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


namespace os {
namespace internal {

string load_entire_file(const char* filename) {
    string result {};

    struct stat st;
    stat(filename, &st);
    usize size = st.st_size;

    void *memory = memory::allocate_pages(size + 1);
    if (memory == NULL) {
        // @todo: handle error.
        return empty_string();
    }

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        // @todo: handle error.
        return empty_string();
    }

    // defer { close(file); };

    usize bytes_read = read(file, memory, size);
    if (bytes_read < size) {
        // @warning: read less than expected!
        return result;
    }
    close(file);

    *((char *)memory + size) = 0; // null-terminate for C API

    result.data = (uint8 *)memory;
    result.size = size;

    return result;
}


bool write_file(const char* filename, string file) {
    INCOMPLETE;
    return false;
}


} // namespace internal
} // namespace os
