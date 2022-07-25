#include "code_location.hpp"


CodeLocation make_code_location(char const *file, int32 l)
{
    CodeLocation cl = {};
    cl.filename = file;
    cl.line = l;
    return cl;
}

CodeLocation make_code_location(char const *file, int32 l, char const *func)
{
    CodeLocation cl = {};
    cl.filename = file;
    cl.line = l;
    cl.function = func;
    return cl;
}

AllocationLogEntry null_allocation_entry()
{
    AllocationLogEntry result = {};
    return result;
}
