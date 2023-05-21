#ifndef HASH_H_
#define HASH_H_

#include "base.h"

// namespace neko_base {

// uint64_t Hash() {}

// }  // namespace neko_base

class Hasher {
 public:
  virtual ~Hasher() = default;

  virtual uint64_t Hash(const char* key, int size, uint64_t seed) = 0;
};

// 64-bit hash for 64-bit platforms
// https://github.com/aappleby/smhasher
class Murmur2 : public Hasher {
 public:
  virtual uint64_t Hash(const char* key, int size, uint64_t seed) override {
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^ (size * m);

    const uint64_t* data = (const uint64_t*)key;
    const uint64_t* end = data + (size / 8);

    while (data != end) {
      uint64_t k = *data++;

      k *= m;
      k ^= k >> r;
      k *= m;

      h ^= k;
      h *= m;
    }

    const unsigned char* data2 = (const unsigned char*)data;

    switch (size & 7) {
      case 7:
        h ^= uint64_t(data2[6]) << 48;
      case 6:
        h ^= uint64_t(data2[5]) << 40;
      case 5:
        h ^= uint64_t(data2[4]) << 32;
      case 4:
        h ^= uint64_t(data2[3]) << 24;
      case 3:
        h ^= uint64_t(data2[2]) << 16;
      case 2:
        h ^= uint64_t(data2[1]) << 8;
      case 1:
        h ^= uint64_t(data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
  }
};

#endif