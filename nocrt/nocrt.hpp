extern "C"
{

int _fltused;

#pragma function(memset)
void *memset(void *destination, int c, size_t count)
{
    char *dst = (char *)destination;
    while (count--) { *dst++ = (char)c; }
    return destination;
}

#pragma function(memcpy)
void *memcpy(void *destination, void const *source, size_t count)
{
    auto dst = (char *)  destination;
    auto src = (char const *) source;
    while (count--) { *dst++ = *src++; }
    return destination;
}

} // extern "C"
