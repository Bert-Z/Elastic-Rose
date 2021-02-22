#pragma once

#include <string>
#include <vector>

#include "hash.hpp"

using namespace std;

namespace elastic_rose
{
    class Elastic_BF
    {
    public:
        Elastic_BF(vector<u64> bits_per_keys, const u64 nkeys)
        {
            size_ = bits_per_keys.size();
            serialized_size_ = 0;
            open_size_ = size_;
            for (u32 i = 0; i < size_; ++i)
                bf_vec_.emplace_back(BF(bits_per_keys[i], nkeys));
        }
        Elastic_BF() = default;
        ~Elastic_BF() = default;

        u64 serializedSize() const
        {
            u64 size = sizeof(size_) + sizeof(serialized_size_);
            for (u32 i = 0; i < size_; ++i)
                size += bf_vec_[i].serializedSize();
            sizeAlign(size);
            return size;
        }

        void serialize(char *&dst)
        {
            memcpy(dst, &size_, sizeof(size_));
            dst += sizeof(size_);

            u32 serialized_size = serializedSize();
            memcpy(dst, &serialized_size, sizeof(serialized_size));
            dst += sizeof(serialized_size);

            for (u32 i = 0; i < size_; ++i)
                bf_vec_[i].serialize(dst);

            align(dst);
        }

        static Elastic_BF *deserialize(char *&src, u32 open_size)
        {
            Elastic_BF *new_ebf = new Elastic_BF();
            memcpy(&(new_ebf->size_), src, sizeof(new_ebf->size_));
            src += sizeof(new_ebf->size_);
            memcpy(&(new_ebf->serialized_size_), src, sizeof(new_ebf->serialized_size_));
            src += sizeof(new_ebf->serialized_size_);

            char *tmp_src = src;

            new_ebf->open_size_ = open_size;
            for (u32 i = 0; i < open_size; ++i)
                new_ebf->bf_vec_.emplace_back(*BF::deserialize(tmp_src));

            src += new_ebf->serialized_size_;
            align(src);
            return new_ebf;
        }

        void add(string &key)
        {
            for (u32 i = 0; i < size_; ++i)
                bf_vec_[i].add(BloomHashId(key, i % 12));
        }

        bool test(string &key)
        {
            for (u32 i = 0; i < open_size_; ++i)
            {
                if (!bf_vec_[i].test(BloomHashId(key, i % 12)))
                    return false;
            }

            return true;
        }

    public:
        vector<BF> bf_vec_;
        u32 size_;
        u32 serialized_size_;

        // not serialized
        u32 open_size_;
    };
} // namespace elastic_rose
