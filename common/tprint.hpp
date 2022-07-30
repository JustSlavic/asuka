#pragma once

#include <string.hpp>
#include <allocator.hpp>


GLOBAL memory::arena_allocator temporary_storage;


string tprint(char const *fmt, ...)
{
    string result = {};

    return result;
}
