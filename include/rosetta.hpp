#pragma once

#include "lib.hpp"
#include "bitvec.hpp"
#include <string>
#include <vector>

#include <iostream>
#include "hash.hpp"

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

        Rosetta(std::vector<std::string> keys, u32 num)
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
                insertKey(str2BitArray(key));
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
        void insertKey(Bitvec<64> key)
        {
            for (u32 i = 0; i < levels; ++i)
            {

                bf_add(bfs[i], BloomHash(key.to_string().substr(0, i + 2)));
            }
        }

        bool lookupKey(u64 key)
        {
            return bf_test(bfs[levels - 1], hash(key));
        }

        bool lookupKey(std::string key)
        {

            return bf_test(bfs[levels - 1], BloomHash(Bitvec<64>(str2BitArray(key)).to_string()));
        }

        bool range_query(u64 low, u64 high, u64 p = 0, u64 l = 1)
        {
            const u64 pow = (1lu << (levels - l + 1));
            // std::cout << "range"
            //           << " low:" << low
            //           << " high:" << high
            //           << " p:" << p
            //           << " l:" << l
            //           //   << " pow:" << pow
            //           << std::endl;

            // std::cout << "(p > high) || ((p + (pow - 1)) < low) " << ((p > high) || ((p + (pow - 1)) < low)) << std::endl;
            // std::cout << "(p >= low) && ((p + (pow - 1)) <= high) " << ((p >= low) && ((p + (pow - 1)) <= high)) << std::endl;

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

        bool range_query(std::string low, std::string high)
        {
            return range_query(Bitvec<64>(str2BitArray(low)), Bitvec<64>(str2BitArray(high)), Bitvec<64>(), 1);
        }

        bool range_query(Bitvec<64> low, Bitvec<64> high, Bitvec<64> p, u64 l)
        {
            std::string pow = "1";
            for (u32 i = 0; i < levels - l + 1; ++i)
                pow += '0';

            std::string pow_1;
            for (u32 i = 0; i < levels - l + 1; ++i)
                pow_1 += '1';

            std::string upper_bound = p.to_string().substr(0, levels + 1 - pow_1.size()) + pow_1;
            if ((p > high) || (low > upper_bound))
            {
                return false;
            }

            if ((p >= low) && (high >= upper_bound))
            {
                return doubt(p, l);
            }

            if (range_query(low, high, p, l + 1))
            {
                return true;
            }

            p[levels - l] = 1;
            return range_query(low, high, p, l + 1);
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

        bool doubt(Bitvec<64> p, u64 l)
        {
            if (l == 1)
            {
                return true;
            }
            if (l > levels + 1)
            {
                return true;
            }

            if (!bf_test(bfs[l - 2], BloomHash(p.to_string())))
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
            p[levels - l + 1] = 1;
            return doubt(p, l + 1);
        }

        std::string str2BitArray(std::string str)
        {
            std::string ret = "";
            for (auto c : str)
                for (int i = 7; i >= 0; --i)
                    ret = (((c >> i) & 1) ? '1' : '0') + ret;
            return ret;
        }
    };
} // namespace elastic_rose
