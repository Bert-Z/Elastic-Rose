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

#include "config.hpp"

#if defined(__x86_64__)
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_acle.h>
#include <arm_neon.h>
#endif

namespace elastic_rose
{
  static inline u64
  bits_round_up(const u64 v, const u8 power)
  {
    return (v + (1lu << power) - 1lu) >> power << power;
  }

  static inline u64
  bits_rotl_u64(const u64 v, const u8 n)
  {
    const u8 sh = n & 0x3f;
    return (v << sh) | (v >> (64 - sh));
  }

  struct bitmap
  {
    u64 bits;
    u64 ones;
    u64 bm[];
  };

  static inline struct bitmap *
  bitmap_create(const u64 bits)
  {
    struct bitmap *const bm = (struct bitmap *const)calloc(1, sizeof(*bm) + (bits_round_up(bits, 6) >> 3));
    bm->bits = bits;
    bm->ones = 0;
    return bm;
  }

  static inline bool
  bitmap_test(const struct bitmap *const bm, const u64 idx)
  {
    return (idx < bm->bits) && (bm->bm[idx >> 6] & (1lu << (idx & 0x3flu)));
  }

  static inline bool
  bitmap_test_all1(struct bitmap *const bm)
  {
    return bm->ones == bm->bits;
  }

  static inline bool
  bitmap_test_all0(struct bitmap *const bm)
  {
    return bm->ones == 0;
  }

  static inline void
  bitmap_set1(struct bitmap *const bm, const u64 idx)
  {
    if ((idx < bm->bits) && !bitmap_test(bm, idx))
    {
      assert(bm->ones < bm->bits);
      bm->bm[idx >> 6] |= (1lu << (idx & 0x3flu));
      bm->ones++;
    }
  }

  static inline void
  bitmap_set0(struct bitmap *const bm, const u64 idx)
  {
    if ((idx < bm->bits) && bitmap_test(bm, idx))
    {
      assert(bm->ones && (bm->ones <= bm->bits));
      bm->bm[idx >> 6] &= ~(1lu << (idx & 0x3flu));
      bm->ones--;
    }
  }

  static inline u64
  bitmap_count(struct bitmap *const bm)
  {
    return bm->ones;
  }

  static inline void
  bitmap_set_all1(struct bitmap *const bm)
  {
    memset(bm->bm, 0xff, bits_round_up(bm->bits, 6) >> 3);
    bm->ones = bm->bits;
  }

  static inline void
  bitmap_set_all0(struct bitmap *const bm)
  {
    memset(bm->bm, 0, bits_round_up(bm->bits, 6) >> 3);
    bm->ones = 0;
  }

  static inline void
  bitmap_static_init(struct bitmap *const bm, const u64 bits)
  {
    bm->bits = bits;
    bitmap_set_all0(bm);
  }

  inline u32
  crc32c_u64(const u32 crc, const u64 v)
  {
#if defined(__x86_64__)
    return (u32)_mm_crc32_u64(crc, v);
#elif defined(__aarch64__)
    return (u32)__crc32cd(crc, v);
#endif
  }

  struct bf
  {
    u64 nr_probe;
    struct bitmap bitmap;
  };

  struct bf *
  bf_create(const u64 bpk, const u64 nkeys)
  {
    const u64 nbits = bpk * nkeys;
    struct bf *const bf = (struct bf *const)malloc(sizeof(*bf) + (bits_round_up(nbits, 6) >> 3));
    bf->nr_probe = (u64)(log(2.0) * (double)bpk);
    bitmap_static_init(&(bf->bitmap), nbits);
    return bf;
  }

  u64 bf_serialized_size(struct bf *const bf)
  {
    u64 size = sizeof(bf->nr_probe) + sizeof(bf->bitmap.bits) + sizeof(bf->bitmap.ones);
    size += bits_round_up(bf->bitmap.bits, 6) >> 3;
    sizeAlign(size);
    return size;
  }

  void bf_serialize(struct bf *const bf, char *&dst)
  {
    memcpy(dst, &bf->nr_probe, sizeof(bf->nr_probe));
    dst += sizeof(bf->nr_probe);
    memcpy(dst, &bf->bitmap.bits, sizeof(bf->bitmap.bits));
    dst += sizeof(bf->bitmap.bits);
    memcpy(dst, &bf->bitmap.ones, sizeof(bf->bitmap.ones));
    dst += sizeof(bf->bitmap.ones);

    u64 bm_size = bits_round_up(bf->bitmap.bits, 6) >> 3;
    memcpy(dst, bf->bitmap.bm, bm_size);
    dst += bm_size;
    align(dst);
  }

  struct bf *bf_deserialize(char *&src)
  {
    u64 nr_probe, nbits, ones;
    memcpy(&nr_probe, src, sizeof(nr_probe));
    src += sizeof(nr_probe);
    memcpy(&nbits, src, sizeof(nbits));
    src += sizeof(nbits);
    memcpy(&ones, src, sizeof(ones));
    src += sizeof(ones);
    u64 bm_size = bits_round_up(nbits, 6) >> 3;
    struct bf *const bf = (struct bf *const)malloc(sizeof(*bf) + bm_size);

    bf->nr_probe = nr_probe;
    bf->bitmap.bits = nbits;
    bf->bitmap.ones = ones;

    memcpy(&(bf->bitmap.bm), src, bm_size);
    src += bm_size;
    align(src);

    return bf;
  }

  static inline u64
  bf_inc(const u64 hash)
  {
    return bits_rotl_u64(hash, 17);
  }

  void bf_add(struct bf *const bf, u64 hash64)
  {
    u64 t = hash64;
    const u64 inc = bf_inc(hash64);
    const u64 bits = bf->bitmap.bits;
    for (u64 i = 0; i < bf->nr_probe; i++)
    {
      bitmap_set1(&(bf->bitmap), t % bits);
      t += inc;
    }
  }

  bool bf_test(const struct bf *const bf, u64 hash64)
  {
    u64 t = hash64;
    const u64 inc = bf_inc(hash64);
    const u64 bits = bf->bitmap.bits;
    for (u64 i = 0; i < bf->nr_probe; i++)
    {
      if (!bitmap_test(&(bf->bitmap), t % bits))
        return false;
      t += inc;
    }
    return true;
  }

  void bf_clean(struct bf *const bf)
  {
    bitmap_set_all0(&(bf->bitmap));
  }

  void bf_destroy(struct bf *const bf)
  {
    free(bf);
  }

} // namespace elastic_rose
