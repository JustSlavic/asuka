#pragma once

#include <defines.hpp>


#define CODE_LOCATION make_code_location(__FILE__, __LINE__)
#define CODE_LOCATION_FUNC make_code_location(__FILE__, __LINE__, __FUNCTION__)

struct CodeLocation
{
    char const *function;
    char const *filename;
    int32 line;
};

CodeLocation make_code_location(char const *file, int32 line)
{
    CodeLocation cl = {};
    cl.filename = file;
    cl.line = line;
    return cl;
}

CodeLocation make_code_location(char const *file, int32 line, char const *func)
{
    CodeLocation cl = {};
    cl.filename = file;
    cl.line = line;
    cl.function = func;
    return cl;
}
