#pragma once

#include <defines.hpp>


struct CodeLocation
{
    char const *function;
    char const *filename;
    int32 line;
};

CodeLocation make_code_location(char const *file, int32 l);
CodeLocation make_code_location(char const *file, int32 l, char const *func);

#define CODE_LOCATION make_code_location(__FILE__, __LINE__)
#define CODE_LOCATION_FUNC make_code_location(__FILE__, __LINE__, __FUNCTION__)

struct AllocationLogEntry
{
    CodeLocation cl;
    void *pointer;
    usize size;
    usize index;
};

AllocationLogEntry null_allocation_entry();

#if UNITY_BUILD
#include "code_location.cpp"
#endif // UNITY_BUILD
