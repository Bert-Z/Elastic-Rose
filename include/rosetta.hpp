#pragma once

#include <string>
#include <vector>

#include <iostream>
#include <bitset>
#include "hash.hpp"

namespace elastic_rose
{
    class Rosetta
    {
    public:
        Rosetta(){};
        Rosetta(std::vector<u64> keys, u32 num) : levels_(64)
        {
            bfs = std::vector<BF *>(levels_);
            // malloc
            for (u32 i = 0; i < levels_; ++i)
                bfs[i] = new BF(10, num);

            // insert keys
            for (auto key : keys)
                insertKey(key);
        }

        Rosetta(std::vector<std::string> keys, u32 num, u32 levels)
        {
            levels_ = levels;
            bfs = std::vector<BF *>(levels_);
            // malloc
            for (u32 i = 0; i < levels_; ++i)
                bfs[i] = new BF(10, num);

            // insert keys
            for (auto key : keys)
                insertKey(key);
        }

        ~Rosetta()
        {
            for (auto bf : bfs)
                delete bf;
        }

        void insertKey(u64 key);
        void insertKey(std::string key);

        bool lookupKey(u64 key);
        bool lookupKey(std::string key);

        bool range_query(u64 low, u64 high, u64 p = 0, u64 l = 1);
        bool range_query(std::string low, std::string high);
        bool range_query(std::string low, std::string high, std::string p, u64 l, std::string& min_accept);

        std::string seek(const std::string& key);

        u64 serializedSize() const;
        char *serialize();
        static Rosetta *deSerialize(char *src);

    private:
        std::vector<BF *> bfs;
        u32 levels_;

        bool doubt(u64 p, u64 l);
        bool doubt(std::string p, u64 l, std::string& min_accept);

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

        std::string bitArray2Str(std::string str){
            std::string ret="";
            size_t size=str.size();
            for(size_t i=0;i<size;i=i+8){
                bitset<8> bit(str.substr(i,8));
                int tmp=bit.to_ullong();
                if(tmp!=0)
                    ret+=static_cast<char>(tmp);
            }
            return ret;
        }

        std::string maxBitArray(){
            std::string ret = "";
            while (ret.size() < levels_)
                ret += '1';

            return ret;
        }
    };

    void Rosetta::insertKey(u64 key)
    {
        for (u32 i = 0; i < levels_; ++i)
        {
            u64 mask = ~((1ul << (levels_ - i - 1)) - 1);
            u64 ik = key & mask;
            bfs[i]->add(u64hash(ik));
        }
    }
    void Rosetta::insertKey(std::string key)
    {
        key = str2BitArray(key);
        for (u32 i = 0; i < levels_; ++i)
            bfs[i]->add(BloomHash(key.substr(0, i + 1)));
    }

    bool Rosetta::lookupKey(u64 key)
    {
        return bfs[levels_ - 1]->test(u64hash(key));
    }

    bool Rosetta::lookupKey(std::string key)
    {
        // std::cout << str2BitArray(key) << std::endl;
        key = str2BitArray(key);
        return bfs[levels_ - 1]->test(BloomHash(key));
    }

    bool Rosetta::range_query(u64 low, u64 high, u64 p, u64 l)
    {
        // std::cout << p << ' ' << l << std::endl;
        const u64 pow_1 = (l == 1) ? UINT64_MAX : ((1lu << (levels_ - l + 1)) - 1);
        const u64 pow_r_1 = 1lu << (levels_ - l);

        if ((p > high) || ((p + pow_1) < low))
        {
            // p is not contained in the range
            return false;
        }

        if ((p >= low) && ((p + pow_1) <= high))
        {
            // p is contained in the range
            return doubt(p, l);
        }

        if (range_query(low, high, p, l + 1))
        {
            return true;
        }

        return range_query(low, high, p + pow_r_1, l + 1);
    }

    bool Rosetta::range_query(std::string low, std::string high)
    {
        std::string p(levels_, '0');
        std::string tmp;
        return range_query(str2BitArray(low), str2BitArray(high), p, 1,tmp);
    }

    bool Rosetta::range_query(std::string low, std::string high, std::string p, u64 l, std::string& min_accept)
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
            return doubt(p, l,min_accept);
        }

        if (range_query(low, high, p, l + 1,min_accept))
        {
            return true;
        }

        p[l - 1] = '1';
        return range_query(low, high, p, l + 1,min_accept);
    }

    bool Rosetta::doubt(u64 p, u64 l)
    {
        // std::cout << "doubt:" << p << ' ' << l << std::endl;
        if (!bfs[l - 2]->test(u64hash(p)))
            return false;

        if (l > levels_)
            return true;

        if (doubt(p, l + 1))
            return true;

        return doubt(p + (1 << (levels_ - l)), l + 1);
    }

    bool Rosetta::doubt(std::string p, u64 l, std::string& min_accept)
    {
        // std::cout << "doubt:" << p << ' ' << l << std::endl;
        if (!bfs[l - 2]->test(BloomHash(p.substr(0, l - 1))))
        {
            return false;
        }

        if (l > levels_){
            min_accept= bitArray2Str(p);
            return true;
        }

        if (doubt(p, l + 1,min_accept))
        {
            return true;
        }
        p[l - 1] = '1';
        return doubt(p, l + 1,min_accept);
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
        rosetta->bfs = std::vector<BF *>(levels);
        for (u32 i = 0; i < levels; ++i)
            rosetta->bfs[i] = BF::deserialize(src);
        align(src);
        return rosetta;
    }

    std::string Rosetta::seek(const std::string& key){
        std::string p(levels_, '0');
        std::string tmp;
        
        return range_query(str2BitArray(key), maxBitArray(), p, 1,tmp)?tmp:"";
    }
} // namespace elastic_rose
