#pragma once

#include <string>
#include <vector>

#include <assert.h>
#include "bloom_filter.hpp"

using namespace std;

namespace elastic_rose
{
    class Elastic_BF
    {
    public:
        Elastic_BF(const std::vector<std::string> &keys, vector<u64> bits_per_keys)
        {
            size_ = bits_per_keys.size();
            serialized_size_ = 0;
            open_size_ = size_;
            for (u32 i = 0; i < size_; ++i)
                bf_vec_.emplace_back(new BloomFilter(keys, bits_per_keys[i], i % 12));
        }

        Elastic_BF(const std::vector<u64> &keys, vector<u64> bits_per_keys)
        {
            size_ = bits_per_keys.size();
            serialized_size_ = 0;
            open_size_ = size_;
            for (u32 i = 0; i < size_; ++i)
                bf_vec_.emplace_back(new BloomFilter(keys, bits_per_keys[i], i % 12));
        }

        Elastic_BF() = default;

        ~Elastic_BF()
        {
            for (auto bf : bf_vec_)
                delete bf;
        }

        u64 serializedSize() const
        {
            u64 size = sizeof(size_) + sizeof(serialized_size_);
            for (u32 i = 0; i < size_; ++i)
                size += bf_vec_[i]->serializedSize();
            sizeAlign(size);
            return size;
        }

        void serialize(char *&dst)
        {
            char *ns = dst;
            memcpy(dst, &size_, sizeof(size_));
            dst += sizeof(size_);

            u32 serialized_size = serializedSize();
            memcpy(dst, &serialized_size, sizeof(serialized_size));
            dst += sizeof(serialized_size);

            for (u32 i = 0; i < size_; ++i)
                bf_vec_[i]->serialize(dst);

            align(dst);
            assert(dst - ns == (int)serializedSize());
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
                new_ebf->bf_vec_.emplace_back(BloomFilter::deserialize(tmp_src));

            src += new_ebf->serialized_size_;
            align(src);
            return new_ebf;
        }

        bool test(const string &key)
        {
            for (u32 i = 0; i < open_size_; ++i)
            {
                if (!bf_vec_[i]->test(key))
                    return false;
            }

            return true;
        }

        bool test(const u64 &key)
        {
            for (u32 i = 0; i < open_size_; ++i)
            {

                if (!bf_vec_[i]->test(key))
                    return false;
            }

            return true;
        }

    public:
        vector<BloomFilter *> bf_vec_;
        u32 size_;
        u32 serialized_size_;

        // not serialized
        u32 open_size_;
    };
} // namespace elastic_rose
