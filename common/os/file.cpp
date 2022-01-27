#include "file.hpp"


#if ASUKA_OS_WINDOWS
#include "windows/file.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/file.hpp"
#endif


namespace os {

//
// Cross-platform Code
//

string load_entire_file(const char* filename) {
    return internal::load_entire_file(filename);
}

bool write_file(const char* filename, string file) {
    return internal::write_file(filename, file);
}


} // namespace os
