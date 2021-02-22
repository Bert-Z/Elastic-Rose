/*
 * Copyright (c) 2016--2020  Wu, Xingbo <wuxb45@gmail.com>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#pragma once
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>

#include "configuration.hpp"

#if defined(__x86_64__)
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_acle.h>
#include <arm_neon.h>
#endif

using namespace std;

namespace elastic_rose
{
  inline u32 crc32c_u64(const u32 crc, const u64 v)
  {
#if defined(__x86_64__)
    return (u32)_mm_crc32_u64(crc, v);
#elif defined(__aarch64__)
    return (u32)__crc32cd(crc, v);
#endif
  }

  static inline u64 bits_round_up(const u64 v, const u8 power)
  {
    return (v + (1lu << power) - 1lu) >> power << power;
  }

  static inline u64 bits_rotl_u64(const u64 v, const u8 n)
  {
    const u8 sh = n & 0x3f;
    return (v << sh) | (v >> (64 - sh));
  }

  class bitmap
  {
  public:
    bitmap() = default;
    bitmap(u64 bits)
        : bits_(bits), ones_(0), bm_(vector<u64>((bits_round_up(bits, 6) >> 3) >> 3, 0)){};
    bitmap(u64 bits, u64 ones, vector<u64> bm)
        : bits_(bits), ones_(ones), bm_(bm){};

    ~bitmap() = default;

    bool test(const u64 idx) const
    {
      return (idx < bits_) && (bm_[idx >> 6] & (1lu << (idx & 0x3flu)));
    }

    bool testAll1() const
    {
      return ones_ == bits_;
    }

    bool testAll0() const
    {
      return ones_ == 0;
    }

    void set1(const u64 idx)
    {
      if ((idx < bits_) && !test(idx))
      {
        assert(ones_ < bits_);
        bm_[idx >> 6] |= (1lu << (idx & 0x3flu));
        ones_++;
      }
    }

    void setAll0()
    {
      ones_ = 0;
      bm_ = vector<u64>((bits_round_up(bits_, 6) >> 3) >> 3, 0);
    }

    u64 count() const
    {
      return ones_;
    }

  public:
    u64 bits_;
    u64 ones_;
    vector<u64> bm_;
  };

  class BF
  {
  public:
    BF(const u64 bpk, const u64 nkeys)
        : nr_probe_((u64)(log(2.0) * (double)bpk)), bitmap_((u64)(bpk * nkeys)){};

    BF(u64 nr_probe, bitmap bm) : nr_probe_(nr_probe), bitmap_(bm){};

    ~BF() = default;

    u64 serializedSize() const
    {
      u64 size = sizeof(nr_probe_) + sizeof(bitmap_.bits_) + sizeof(bitmap_.ones_);
      size += bitmap_.bm_.size() * sizeof(u64);
      sizeAlign(size);
      return size;
    }

    void serialize(char *&dst)
    {
      memcpy(dst, &nr_probe_, sizeof(nr_probe_));
      dst += sizeof(nr_probe_);
      memcpy(dst, &(bitmap_.bits_), sizeof(bitmap_.bits_));
      dst += sizeof(bitmap_.bits_);
      memcpy(dst, &(bitmap_.ones_), sizeof(bitmap_.ones_));
      dst += sizeof(bitmap_.ones_);

      u64 bm_size = bitmap_.bm_.size() * sizeof(u64);
      memcpy(dst, bitmap_.bm_.data(), bm_size);
      dst += bm_size;
      align(dst);
    }

    static BF *deserialize(char *&src)
    {
      u64 nr_probe, nbits, ones;
      memcpy(&nr_probe, src, sizeof(nr_probe));
      src += sizeof(nr_probe);
      memcpy(&nbits, src, sizeof(nbits));
      src += sizeof(nbits);
      memcpy(&ones, src, sizeof(ones));
      src += sizeof(ones);
      u64 bm_size = bits_round_up(nbits, 6) >> 3;

      bitmap bm(nbits, ones, vector<u64>((u64 *)src, (u64 *)src + (bm_size >> 3)));
      BF *new_bf = new BF(nr_probe, bm);
      src += bm_size;
      align(src);

      return new_bf;
    }

    u64 bf_inc(const u64 hash)
    {
      return bits_rotl_u64(hash, 17);
    }

    void add(u64 hash64)
    {
      u64 t = hash64;
      const u64 inc = bf_inc(hash64);
      const u64 bits = bitmap_.bits_;
      for (u64 i = 0; i < nr_probe_; i++)
      {
        bitmap_.set1(t % bits);
        t += inc;
      }
    }

    bool test(u64 hash64)
    {
      u64 t = hash64;
      const u64 inc = bf_inc(hash64);
      const u64 bits = bitmap_.bits_;
      for (u64 i = 0; i < nr_probe_; i++)
      {
        if (!bitmap_.test(t % bits))
          return false;
        t += inc;
      }
      return true;
    }

    void clean()
    {
      bitmap_.setAll0();
    }

  public:
    u64 nr_probe_;
    bitmap bitmap_;
  };

  struct bf
  {
    u64 nr_probe;
    struct bitmap bitmap;
  };

} // namespace elastic_rose
