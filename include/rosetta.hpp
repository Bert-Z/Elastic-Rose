#pragma once

#include "lib.hpp"
#include <string>
#include <vector>

#include <iostream>

namespace elastic_rose
{
    class Rosetta
    {
    public:
        Rosetta(){};
        Rosetta(std::vector<u64> keys, u32 num)
        {
            // FIXME: solid make levels as 63 for u64 now
            levels = 63;
            bfs = std::vector<struct bf *>(levels);
            // malloc
            for (u32 i = 0; i < levels; ++i)
            {
                struct bf *bf = bf_create(10, num);
                assert(bf);
                bfs[i] = bf;
            }

            // insert keys
            for (auto key : keys)
                insertKey(key);
        }

        ~Rosetta()
        {
            for (auto bf : bfs)
                bf_destroy(bf);
        }

        void insertKey(u64 key)
        {
            for (u32 i = 0; i < levels; ++i)
            {
                u64 mask = ~((1ul << (levels - i - 1)) - 1);
                u64 ik = key & mask;
                bf_add(bfs[i], hash(ik));
            }
        }

        bool lookupKey(u64 key)
        {
            return bf_test(bfs[levels - 1], hash(key));
        }

        bool range_query(u64 low, u64 high, u64 p = 0, u64 l = 1)
        {
            const u64 pow = (1lu << (levels - l + 1));

            if ((p > high) || ((p + (pow - 1)) < low))
            {
                // p is not contained in the range
                return false;
            }

            if ((p >= low) && ((p + (pow - 1)) <= high))
            {
                // p is contained in the range
                return doubt(p, l);
            }

            if (range_query(low, high, p, l + 1))
            {
                return true;
            }

            return range_query(low, high, p + (pow >> 1), l + 1);
        }

    private:
        std::vector<struct bf *> bfs;
        u32 levels;

        u64 hash(u64 key)
        {
            u32 lo = crc32c_u64(0xDEADBEEF, key);
            return (u64)lo << 32 | lo;
        }

        bool doubt(u64 p, u64 l)
        {
            if (l == 1)
            {
                return true;
            }
            if (l > levels + 1)
            {
                return true;
            }

            if (!bf_test(bfs[l - 2], hash(p)))
            {
                return false;
            }

            if (l > levels)
            {
                return true;
            }

            if (doubt(p, l + 1))
            {
                return true;
            }
            return doubt(p + (1 << (levels - l)), l + 1);
        }
    };
} // namespace elastic_rose
