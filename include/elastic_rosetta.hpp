#pragma once

#include <string>
#include <vector>

#include <iostream>
#include <bitset>
#include "elastic_bf.hpp"

namespace elastic_rose
{
    class Elastic_Rosetta
    {
    public:
        Elastic_Rosetta(){};

        Elastic_Rosetta(std::vector<std::string> keys, u32 num, u32 levels, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
        {
            levels_ = levels;
            bfs_ = std::vector<BF *>(levels_ - 1);
            // malloc
            for (u32 i = 0; i < levels_ - 1; ++i)
                bfs_[i] = new BF(bits_per_key, num);

            last_level_ebf_ = new Elastic_BF(last_level_bits_per_keys, num);

            // insert keys
            for (auto key : keys)
                insertKey(key);
        }

        ~Elastic_Rosetta()
        {
            for (auto bf : bfs_)
                delete bf;
            delete last_level_ebf_;
        }

        void bfAdd(u32 level, std::string key)
        {
            if (level == levels_ - 1)
                last_level_ebf_->add(key);
            else
                bfs_[level]->add(BloomHash(key));
        }

        bool bfTest(u32 level, std::string key)
        {
            if (level == levels_ - 1)
                return last_level_ebf_->test(key);
            else
                return bfs_[level]->test(BloomHash(key));
        }

        void insertKey(std::string key);

        bool lookupKey(std::string key);

        bool range_query(std::string low, std::string high);
        bool range_query(std::string low, std::string high, std::string p, u64 l, std::string &min_accept);

        std::string seek(const std::string &key);

        u64 serializedSize() const;
        char *serialize();
        static Elastic_Rosetta *deSerialize(char *src, u32 open_size);

    private:
        std::vector<BF *> bfs_;
        Elastic_BF *last_level_ebf_;
        u32 levels_;

        bool doubt(u64 p, u64 l);
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
    };

    void Elastic_Rosetta::insertKey(std::string key)
    {
        key = str2BitArray(key);
        for (u32 i = 0; i < levels_; ++i)
            bfAdd(i, key.substr(0, i + 1));
        // bfs_[i]->add(BloomHash(key.substr(0, i + 1)));
    }

    bool Elastic_Rosetta::lookupKey(std::string key)
    {
        // std::cout << str2BitArray(key) << std::endl;
        key = str2BitArray(key);
        // return bfs_[levels_ - 1]->test(BloomHash(key));
        return bfTest(levels_ - 1, key);
    }

    bool Elastic_Rosetta::range_query(std::string low, std::string high)
    {
        std::string p(levels_, '0');
        std::string tmp;
        return range_query(str2BitArray(low), str2BitArray(high), p, 1, tmp);
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
        elastic_rosetta->bfs_ = std::vector<BF *>(levels);
        for (u32 i = 0; i < levels - 1; ++i)
            elastic_rosetta->bfs_[i] = BF::deserialize(src);
        align(src);
        elastic_rosetta->last_level_ebf_ = Elastic_BF::deserialize(src, open_size);
        align(src);

        return elastic_rosetta;
    }

    std::string Elastic_Rosetta::seek(const std::string &key)
    {
        std::string p(levels_, '0');
        std::string tmp;

        return range_query(str2BitArray(key), maxBitArray(), p, 1, tmp) ? tmp : "";
    }
} // namespace elastic_rose
