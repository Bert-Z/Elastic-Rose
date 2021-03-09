#pragma once

#include <cstddef>
#include <time.h>
#include <sys/time.h>

namespace elastic_rose
{
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long;

    inline void align(char *&ptr)
    {
        ptr = (char *)(((uint64_t)ptr + 7) & ~((uint64_t)7));
    }

    inline void sizeAlign(u32 &size)
    {
        size = (size + 7) & ~((u32)7);
    }

    inline void sizeAlign(u64 &size)
    {
        size = (size + 7) & ~((u64)7);
    }

    // for time measurement
    inline double getNow()
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }
} // namespace elastic_rose
