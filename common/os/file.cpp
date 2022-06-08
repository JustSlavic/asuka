#include "file.hpp"


#if ASUKA_OS_WINDOWS
#include "windows/file.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/file.hpp"
#endif


namespace Asuka {
namespace os {

//
// Cross-platform code.
//

byte_array load_entire_file(const char* filename) {
    return internal::load_entire_file(filename);
}

bool write_file(const char* filename, byte_array contents) {
    return internal::write_file(filename, contents);
}


} // namespace os
} // namespace Asuka
