#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <bitset>
#include <assert.h>

#include "bloom_filter.hpp"

namespace elastic_rose
{
    class Rosetta
    {
    public:
        Rosetta(){};

        Rosetta(const std::vector<std::string> &keys, u32 num, u32 levels, u64 bits_per_key)
        {
            levels_ = levels;
            std::cout << "levels:" << levels_ << std::endl;
            bfs = std::vector<BloomFilter *>(levels_);

            std::vector<std::string> key_vec = keys;

            double pre_time1, pre_time2, pre_time = 0, build_time = 0;
            for (int i = levels - 1; i >= 0; --i)
            {
                pre_time1 = getNow();
                for (u32 j = 0; j < num; ++j)
                    key_vec[j] = key_vec[j].substr(0, i + 1);
                pre_time2 = getNow();
                pre_time += pre_time2 - pre_time1;

                pre_time1 = getNow();
                bfs[i] = new BloomFilter(key_vec, bits_per_key);
                pre_time2 = getNow();
                build_time += pre_time2 - pre_time1;
            }

            std::cout << "pre_time:" << pre_time << std::endl;
            std::cout << "bloom_build_time:" << build_time << std::endl;
        }

        ~Rosetta()
        {
            for (auto bf : bfs)
                delete bf;
        }

        bool lookupKey(const std::string &key);

        bool range_query(std::string low, std::string high);
        bool range_query(std::string low, std::string high, std::string p, u64 l, std::string &min_accept);

        std::string seek(const std::string &key);

        u64 serializedSize() const;
        char *serialize();
        static Rosetta *deSerialize(char *src);

    private:
        std::vector<BloomFilter *> bfs;
        u32 levels_;

        bool doubt(std::string p, u64 l, std::string &min_accept);

        std::string str2BitArray(const std::string &str)
        {
            std::string ret = "";
            for (auto c : str)
                for (int i = 7; i >= 0; --i)
                    ret += (((c >> i) & 1) ? '1' : '0');

            // format str size
            while (ret.size() < levels_)
                ret += '0';

            return ret;
        }

        std::string bitArray2Str(std::string &str)
        {
            std::string ret = "";
            size_t size = str.size();
            for (size_t i = 0; i < size; i = i + 8)
            {
                bitset<8> bit(str.substr(i, 8));
                int tmp = bit.to_ullong();
                if (tmp != 0)
                    ret += static_cast<char>(tmp);
            }
            return ret;
        }

        std::string maxBitArray()
        {
            std::string ret = "";
            while (ret.size() < levels_)
                ret += '1';

            return ret;
        }
    };

    bool Rosetta::lookupKey(const std::string &key)
    {
        // std::cout << str2BitArray(key) << std::endl;
        // std::string bit_key = str2BitArray(key);
        return bfs[levels_ - 1]->test(key);
    }

    bool Rosetta::range_query(std::string low, std::string high)
    {
        std::string p(levels_, '0');
        std::string tmp;
        // return range_query(str2BitArray(low), str2BitArray(high), p, 1, tmp);
        return range_query(low, high, p, 1, tmp);
    }

    bool Rosetta::range_query(std::string low, std::string high, std::string p, u64 l, std::string &min_accept)
    {
        // std::cout << p << ' ' << l << std::endl;
        std::string pow_1;
        for (u32 i = 0; i < levels_ - l + 1; ++i)
            pow_1 += '1';

        std::string upper_bound = p.substr(0, levels_ - pow_1.size()) + pow_1;

        if ((p > high) || (low > upper_bound))
        {
            return false;
        }

        if ((p >= low) && (high >= upper_bound))
        {
            return doubt(p, l, min_accept);
        }

        if (range_query(low, high, p, l + 1, min_accept))
        {
            return true;
        }

        p[l - 1] = '1';
        return range_query(low, high, p, l + 1, min_accept);
    }

    bool Rosetta::doubt(std::string p, u64 l, std::string &min_accept)
    {
        // std::cout << "doubt:" << p << ' ' << l << std::endl;
        if (!bfs[l - 2]->test(p.substr(0, l - 1)))
        {
            return false;
        }

        if (l > levels_)
        {
            min_accept = bitArray2Str(p);
            return true;
        }

        if (doubt(p, l + 1, min_accept))
        {
            return true;
        }
        p[l - 1] = '1';
        return doubt(p, l + 1, min_accept);
    }

    u64 Rosetta::serializedSize() const
    {
        u64 size = sizeof(levels_);
        sizeAlign(size);
        for (auto bf : bfs)
            size += bf->serializedSize();
        sizeAlign(size);
        return size;
    }
    char *Rosetta::serialize()
    {
        u64 size = serializedSize();
        char *data = new char[size];
        char *cur_data = data;

        memcpy(cur_data, &levels_, sizeof(levels_));
        cur_data += sizeof(levels_);
        align(cur_data);
        for (u32 i = 0; i < levels_; ++i)
            bfs[i]->serialize(cur_data);
        align(cur_data);
        assert(cur_data - data == (int64_t)size);
        return data;
    }
    Rosetta *Rosetta::deSerialize(char *src)
    {
        u32 levels;
        memcpy(&levels, src, sizeof(levels));
        src += sizeof(levels);
        align(src);
        Rosetta *rosetta = new Rosetta();
        rosetta->levels_ = levels;
        std::cout << rosetta->levels_ << std::endl;
        rosetta->bfs = std::vector<BloomFilter *>(levels);
        for (u32 i = 0; i < levels; ++i)
            rosetta->bfs[i] = BloomFilter::deserialize(src);

        align(src);
        return rosetta;
    }

    std::string Rosetta::seek(const std::string &key)
    {
        std::string p(levels_, '0');
        std::string tmp;

        // return range_query(str2BitArray(key), maxBitArray(), p, 1, tmp) ? tmp : "";
        return range_query(key, maxBitArray(), p, 1, tmp) ? tmp : "";
    }
} // namespace elastic_rose
