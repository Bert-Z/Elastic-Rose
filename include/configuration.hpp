#pragma once

#include <cstddef>

namespace elastic_rose
{
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long;

    static const size_t bits_per_key = 10;

    void align(char *&ptr)
    {
        ptr = (char *)(((uint64_t)ptr + 7) & ~((uint64_t)7));
    }

    void sizeAlign(u32 &size)
    {
        size = (size + 7) & ~((u32)7);
    }

    void sizeAlign(u64 &size)
    {
        size = (size + 7) & ~((u64)7);
    }
} // namespace elastic_rose
