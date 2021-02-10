#pragma once

#include "lib.hpp"
#include <string>
#include <vector>

namespace elastic_rose
{
    class Rosetta
    {
    public:
        Rosetta(){};
        Rosetta(std::vector<u64> keys, u32 num)
            : levels(log2(sizeof(u64) * 8) + 1),
              bfs(std::vector<struct bf *>(levels))
        {
            // malloc
            for (auto i = 0; i < levels; ++i)
            {
                struct bf *bf = bf_create(10, num);
                assert(bf);
                bfs[i] = bf;
            }

            // insert keys
            for (auto key : keys)
                insertKey(key);
        }

        void insertKey(u64 key)
        {
            for (int i = 0; i <= levels; ++i)
            {
                auto ik = preKBits(key, 1 << i);
                bf_add(bfs[i], HASH(ik));
            }
        }

        bool lookupKey(u64 key)
        {
            return bf_test(bfs[levels], HASH(key));
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

        u64 preKBits(u64 key, u32 k)
        {
            u32 m = 64 - k;
            return key >> m;
        }

        u64 HASH(u64 key)
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
            if (l > (levels + 1))
            {
                return true;
            }
            auto bf = bfs[l - 2];

            if (!bf_test(bf, HASH(p)))
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
