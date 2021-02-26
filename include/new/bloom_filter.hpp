// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

// Modified by Huanchen, 2018

#pragma once

#include <stdint.h>
#include <string.h>

#include <vector>
#include <string>

#include "MurmurHash3.h"
#include "../configuration.hpp"

using namespace std;

namespace elastic_rose
{
    static void LeveldbBloomHash(const u64 key, u32 *out, u32 hash_code)
    {
        MurmurHash3_x86_128((const char *)(&key), sizeof(u64), hash_code, out);
    }

    static void LeveldbBloomHash(const string &key, u32 *out, u32 hash_code)
    {
        MurmurHash3_x86_128(key.c_str(), key.size(), hash_code, out);
    }

    inline uint32_t BloomHashId(const std::string &key, uint32_t id)
    {
        u32 ret = 0;
        switch (id)
        {
        case 0:
            LeveldbBloomHash(key, &ret, 0xbc9f1d34);
            return ret;
        case 1:
            LeveldbBloomHash(key, &ret, 0x34f1d34b);
            return ret;
        case 2:
            LeveldbBloomHash(key, &ret, 0x251d34bc);
            return ret;
        case 3:
            LeveldbBloomHash(key, &ret, 0x01d34bc9);
            return ret;
        case 4:
            LeveldbBloomHash(key, &ret, 0x1934bc9f);
            return ret;
        case 5:
            LeveldbBloomHash(key, &ret, 0x934bc9f1);
            return ret;
        case 6:
            LeveldbBloomHash(key, &ret, 0x4bc9f193);
            return ret;
        case 7:
            LeveldbBloomHash(key, &ret, 0x51c2578a);
            return ret;
        case 8:
            LeveldbBloomHash(key, &ret, 0xda23562f);
            return ret;
        case 9:
            LeveldbBloomHash(key, &ret, 0x135254f2);
            return ret;
        case 10:
            LeveldbBloomHash(key, &ret, 0xea1e4a48);
            return ret;
        case 11:
            LeveldbBloomHash(key, &ret, 0x567925f1);
            return ret;
        default:
            fprintf(stderr, "BloomHash id error\n");
            exit(1);
        }
    }

    class Bloom
    {
    private:
        size_t bits_per_key_;
        size_t k_;
        size_t id_;

    public:
        Bloom() = default;
        Bloom(int bits_per_key, u32 id)
            : bits_per_key_(bits_per_key), id_(id)
        {
            // We intentionally round down to reduce probing cost a little bit
            k_ = static_cast<size_t>(bits_per_key * 0.69); // 0.69 =~ ln(2)
            if (k_ < 1)
                k_ = 1;
            if (k_ > 30)
                k_ = 30;
        }

        void CreateFilter(const vector<string> &keys, int n, string *dst) const
        {
            // Compute bloom filter size (in both bits and bytes)
            size_t bits = n * bits_per_key_;

            // For small n, we can see a very high false positive rate.  Fix it
            // by enforcing a minimum bloom filter length.
            if (bits < 64)
                bits = 64;

            size_t bytes = (bits + 7) / 8;
            bits = bytes * 8;

            const size_t init_size = dst->size();
            dst->resize(init_size + bytes, 0);
            dst->push_back(static_cast<char>(k_)); // Remember # of probes in filter
            char *array = &(*dst)[init_size];
            for (int i = 0; i < n; i++)
            {
                // Use double-hashing to generate a sequence of hash values.
                // See analysis in [Kirsch,Mitzenmacher 2006].
                // u32 h = LeveldbBloomHash(keys[i]);
                u32 hbase[4];
                LeveldbBloomHash(keys[i], hbase, id_);
                u32 h = hbase[0];
                const u32 delta = hbase[1];
                for (size_t j = 0; j < k_; j++)
                {
                    const u32 bitpos = h % bits;
                    array[bitpos / 8] |= (1 << (bitpos % 8));
                    h += delta;
                }
            }
        }

        void CreateFilter(vector<u64> keys, int n, string *dst) const
        {
            // Compute bloom filter size (in both bits and bytes)
            size_t bits = n * bits_per_key_;

            // For small n, we can see a very high false positive rate.  Fix it
            // by enforcing a minimum bloom filter length.
            if (bits < 64)
                bits = 64;

            size_t bytes = (bits + 7) / 8;
            bits = bytes * 8;

            const size_t init_size = dst->size();
            dst->resize(init_size + bytes, 0);
            dst->push_back(static_cast<char>(k_)); // Remember # of probes in filter
            char *array = &(*dst)[init_size];
            for (int i = 0; i < n; i++)
            {
                // Use double-hashing to generate a sequence of hash values.
                // See analysis in [Kirsch,Mitzenmacher 2006].
                //u32 h = LeveldbBloomHash(keys[i]);
                u32 hbase[4];
                LeveldbBloomHash(keys[i], hbase, id_);
                u32 h = hbase[0];
                const u32 delta = hbase[1];
                for (size_t j = 0; j < k_; j++)
                {
                    const u32 bitpos = h % bits;
                    array[bitpos / 8] |= (1 << (bitpos % 8));
                    h += delta;
                }
            }
        }

        bool KeyMayMatch(const string &key, const string &bloom_filter) const
        {
            const size_t len = bloom_filter.size();
            if (len < 2)
                return false;

            const char *array = bloom_filter.c_str();
            const size_t bits = (len - 1) * 8;

            // Use the encoded k so that we can read filters generated by
            // bloom filters created using different parameters.
            const size_t k = array[len - 1];
            if (k > 30)
            {
                // Reserved for potentially new encodings for short bloom filters.
                // Consider it a match.
                return true;
            }

            u32 hbase[4];
            LeveldbBloomHash(key, hbase, id_);
            u32 h = hbase[0];
            const u32 delta = hbase[1];
            for (size_t j = 0; j < k; j++)
            {
                const u32 bitpos = h % bits;
                if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0)
                    return false;
                h += delta;
            }
            return true;
        }

        bool KeyMayMatch(const u64 key, const string &bloom_filter) const
        {
            const size_t len = bloom_filter.size();
            if (len < 2)
                return false;

            const char *array = bloom_filter.c_str();
            const size_t bits = (len - 1) * 8;

            // Use the encoded k so that we can read filters generated by
            // bloom filters created using different parameters.
            const size_t k = array[len - 1];
            if (k > 30)
            {
                // Reserved for potentially new encodings for short bloom filters.
                // Consider it a match.
                return true;
            }

            u32 hbase[4];
            LeveldbBloomHash(key, hbase, id_);
            u32 h = hbase[0];
            const u32 delta = hbase[1];
            for (size_t j = 0; j < k; j++)
            {
                const u32 bitpos = h % bits;
                if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0)
                    return false;
                h += delta;
            }
            return true;
        }
    };

    class BloomFilter
    {
    public:
        // Requires that keys are sorted
        BloomFilter() = default;

        BloomFilter(const std::vector<std::string> &keys, const u64 bits_per_key, const u32 id = 0) : id_(id)
        {
            filter_ = new Bloom(bits_per_key, id);
            filter_->CreateFilter(keys, keys.size(), &filter_data_);
        }

        BloomFilter(const std::vector<u64> &keys, const u64 bits_per_key, const u32 id = 0) : id_(id)
        {
            filter_ = new Bloom(bits_per_key, id);
            filter_->CreateFilter(keys, keys.size(), &filter_data_);
        }

        BloomFilter(const u32 filter_data_size, const std::string &filter_data, const u32 id = 0)
            : id_(id), filter_(new Bloom()), filter_data_size_(filter_data_size), filter_data_(filter_data) {}

        ~BloomFilter()
        {
            delete filter_;
        }

        bool test(const std::string &key)
        {
            return filter_->KeyMayMatch(key, filter_data_);
        }

        bool test(const u64 &key)
        {
            return filter_->KeyMayMatch(key, filter_data_);
        }

        u64 getMemoryUsage()
        {
            return filter_data_.size();
        }

        u64 serializedSize() const
        {
            u64 size = sizeof(filter_data_size_) + sizeof(id_) + (filter_data_.size() + 1) * sizeof(char);
            sizeAlign(size);
            return size;
        }

        void serialize(char *&dst)
        {
            filter_data_size_ = filter_data_.size() + 1;
            memcpy(dst, &filter_data_size_, sizeof(filter_data_size_));
            dst += sizeof(filter_data_size_);
            memcpy(dst, &id_, sizeof(id_));
            dst += sizeof(id_);
            memcpy(dst, filter_data_.c_str(), filter_data_size_ * sizeof(char));
            dst += filter_data_size_ * sizeof(char);
            align(dst);
        }

        static BloomFilter *deserialize(char *&src)
        {
            u32 filter_data_size, id;
            memcpy(&filter_data_size, src, sizeof(filter_data_size));
            src += sizeof(filter_data_size);
            memcpy(&id, src, sizeof(id));
            src += sizeof(id);

            char *filter_data = new char[filter_data_size];
            memcpy(filter_data, src, filter_data_size * sizeof(char));
            src += filter_data_size;
            align(src);

            std::string filter_data_str = std::string(filter_data);
            BloomFilter *new_bf = new BloomFilter(filter_data_size, filter_data_str, id);

            return new_bf;
        }

    private:
        u32 id_;
        Bloom *filter_;
        u32 filter_data_size_;
        std::string filter_data_;
    };

} // namespace elastic_rose
