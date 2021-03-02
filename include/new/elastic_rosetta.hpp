#pragma once

#include <string>
#include <vector>

#include <iostream>
#include <bitset>
#include <assert.h>
#include <math.h>
#include "elastic_bf.hpp"

namespace elastic_rose
{
    class Elastic_Rosetta
    {
    public:
        Elastic_Rosetta(){};

        Elastic_Rosetta(const std::vector<std::string> &keys, u32 levels, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
        {
            levels_ = levels;
            u32 num = keys.size();
            bfs_ = std::vector<BloomFilter *>(levels_ - 1);

            std::vector<std::string> key_vec = keys;

            for (int i = levels - 2; i >= 0; --i)
            {
                for (u32 j = 0; j < num; ++j)
                    key_vec[j] = key_vec[j].substr(0, i + 1);

                bfs_[i] = new BloomFilter(key_vec, bits_per_key);
            }
            last_level_ebf_ = new Elastic_BF(keys, last_level_bits_per_keys);
        }

        Elastic_Rosetta(const std::vector<u64> &keys, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys) : levels_(64)
        {
            u32 num = keys.size();
            bfs_ = std::vector<BloomFilter *>(levels_ - 1);

            // key range
            R_ = keys[num - 1] - keys[0];

            std::vector<double> bpk_per_level_vec(levels_);
            if (R_ != 0)
                calBPK(bpk_per_level_vec, bits_per_key);

            std::vector<u64> key_vec = keys;

            for (int i = levels_ - 2; i >= 0; --i)
            {
                u64 mask = ~((1ul << (levels_ - i - 1)) - 1);
                for (u32 j = 0; j < num; ++j)
                    key_vec[j] = key_vec[j] & mask;
                bfs_[i] = new BloomFilter(key_vec, (u64)bpk_per_level_vec[i]);
            }
            last_level_ebf_ = new Elastic_BF(keys, last_level_bits_per_keys);
        }

        ~Elastic_Rosetta()
        {
            for (auto bf : bfs_)
                delete bf;
            delete last_level_ebf_;
        }

        bool bfTest(u32 level, std::string key)
        {
            if (level == levels_ - 1)
                return last_level_ebf_->test(key);
            else
                return bfs_[level]->test(key);
        }

        bool bfTest(u32 level, u64 key)
        {
            if (level == levels_ - 1)
                return last_level_ebf_->test(key);
            else
                return bfs_[level]->test(key);
        }

        bool lookupKey(const std::string &key);
        bool lookupKey(const u64 &key);

        bool range_query(u64 low, u64 high);
        bool range_query(u64 low, u64 high, u64 &min_accept, u64 p = 0, u64 l = 1);
        bool range_query(std::string low, std::string high);
        bool range_query(std::string low, std::string high, std::string p, u64 l, std::string &min_accept);

        std::string seek(const std::string &key);
        u64 seek(const u64 &key);

        u64 serializedSize() const;
        char *serialize();
        static Elastic_Rosetta *deSerialize(char *src, u32 open_size);

    private:
        std::vector<BloomFilter *> bfs_;
        Elastic_BF *last_level_ebf_;
        u32 levels_;

        u64 R_;

        bool doubt(u64 p, u64 l, u64 &min_accept);
        bool doubt(std::string p, u64 l, std::string &min_accept);

        std::string str2BitArray(std::string str)
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

        std::string bitArray2Str(std::string str)
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

        double g(int x)
        {
            int logR = log(R_);
            if (x < logR)
                return 1;
            else if (x == logR)
                return (double)(R_ - (1 << x) + 1) / (double)(1 << x);
            else
                return 0;
        }

        double levelFrequency(int r)
        {
            int logR = log(R_);
            double ret = 0;
            for (int i = 0; i <= logR - r; ++i)
                ret += g(r + i);
            return ret;
        }

        void calBPK(std::vector<double> &bpk_per_level_vec, u64 bits_per_key)
        {
            double fre_min = 0;
            bool isset = false;
            for (u32 i = 0; i < levels_; ++i)
            {
                bpk_per_level_vec[i] = levelFrequency(levels_ - i - 1);
                if (bpk_per_level_vec[i] != 0 && !isset)
                {
                    fre_min = bpk_per_level_vec[i];
                    isset = true;
                }
            }
            double fre_max = bpk_per_level_vec[levels_ - 1];

            for (u32 i = 0; i < levels_; ++i)
            {
                if (bpk_per_level_vec[i] != 0)
                    bpk_per_level_vec[i] = (bits_per_key / 2) * (2 - (fre_max - bpk_per_level_vec[i]) / (fre_max - fre_min));
            }
        }
    };

    bool Elastic_Rosetta::lookupKey(const std::string &key)
    {
        // std::cout << str2BitArray(key) << std::endl;
        // key = str2BitArray(key);
        // return bfs_[levels_ - 1]->test(BloomHash(key));
        return bfTest(levels_ - 1, key);
    }

    bool Elastic_Rosetta::lookupKey(const u64 &key)
    {
        return bfTest(levels_ - 1, key);
    }

    bool Elastic_Rosetta::range_query(std::string low, std::string high)
    {
        std::string p(levels_, '0');
        std::string tmp;
        // return range_query(str2BitArray(low), str2BitArray(high), p, 1, tmp);
        return range_query(low, high, p, 1, tmp);
    }

    bool Elastic_Rosetta::range_query(u64 low, u64 high)
    {
        u64 tmp = 0;
        return range_query(low, high, tmp, 0, 1);
    }

    bool Elastic_Rosetta::range_query(u64 low, u64 high, u64 &min_accept, u64 p, u64 l)
    {
        const u64 pow_1 = (l == 1) ? UINT64_MAX : ((1lu << (levels_ - l + 1)) - 1);
        const u64 pow_r_1 = 1lu << (levels_ - l);
        // std::cout << "range:" << p << ' ' << l << std::endl;

        if ((p > high) || ((p + pow_1) < low))
        {
            // p is not contained in the range
            return false;
        }

        if ((p >= low) && ((p + pow_1) <= high))
        {
            // p is contained in the range
            return doubt(p, l, min_accept);
        }

        if (range_query(low, high, min_accept, p, l + 1))
        {
            return true;
        }

        return range_query(low, high, min_accept, p + pow_r_1, l + 1);
    }

    bool Elastic_Rosetta::range_query(std::string low, std::string high, std::string p, u64 l, std::string &min_accept)
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

    bool Elastic_Rosetta::doubt(u64 p, u64 l, u64 &min_accept)
    {
        if (!bfTest(l - 2, p))
            return false;

        if (l > levels_)
        {
            min_accept = p;
            return true;
        }

        if (doubt(p, l + 1, min_accept))
            return true;

        return doubt(p + (1lu << (levels_ - l)), l + 1, min_accept);
    }

    bool Elastic_Rosetta::doubt(std::string p, u64 l, std::string &min_accept)
    {
        // std::cout << "doubt:" << p << ' ' << l << std::endl;
        // if (!bfs_[l - 2]->test(BloomHash(p.substr(0, l - 1))))
        if (!bfTest(l - 2, p.substr(0, l - 1)))
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

    u64 Elastic_Rosetta::serializedSize() const
    {
        u64 size = sizeof(levels_);
        sizeAlign(size);
        for (auto bf : bfs_)
            size += bf->serializedSize();
        sizeAlign(size);
        size += last_level_ebf_->serializedSize();
        sizeAlign(size);
        return size;
    }
    char *Elastic_Rosetta::serialize()
    {
        u64 size = serializedSize();
        char *data = new char[size];
        char *cur_data = data;

        memcpy(cur_data, &levels_, sizeof(levels_));
        cur_data += sizeof(levels_);
        align(cur_data);
        for (u32 i = 0; i < levels_ - 1; ++i)
            bfs_[i]->serialize(cur_data);
        align(cur_data);
        last_level_ebf_->serialize(cur_data);
        align(cur_data);
        assert(cur_data - data == (int64_t)size);
        return data;
    }
    Elastic_Rosetta *Elastic_Rosetta::deSerialize(char *src, u32 open_size)
    {
        u32 levels;
        memcpy(&levels, src, sizeof(levels));
        src += sizeof(levels);
        align(src);
        Elastic_Rosetta *elastic_rosetta = new Elastic_Rosetta();
        elastic_rosetta->levels_ = levels;
        elastic_rosetta->bfs_ = std::vector<BloomFilter *>(levels);
        for (u32 i = 0; i < levels - 1; ++i)
            elastic_rosetta->bfs_[i] = BloomFilter::deserialize(src);
        align(src);
        elastic_rosetta->last_level_ebf_ = Elastic_BF::deserialize(src, open_size);
        align(src);

        return elastic_rosetta;
    }

    u64 Elastic_Rosetta::seek(const u64 &key)
    {
        u64 tmp = 0;
        return range_query(key, UINT64_MAX, tmp, 0, 1) ? tmp : 0;
    }

    std::string Elastic_Rosetta::seek(const std::string &key)
    {
        std::string p(levels_, '0');
        std::string tmp;

        return range_query(str2BitArray(key), maxBitArray(), p, 1, tmp) ? tmp : "";
    }
} // namespace elastic_rose
