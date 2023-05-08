#include "variable_int.h"

char* VariableInteger::Encode_uint64(const uint64_t src) {
  int out_byte = kMax_uint64OutBytes;
  uint64_t ul = src;  // little-endian
  char* byte_ptr = reinterpret_cast<char*>(&ul) + sizeof(ul) - 1;

  if (!(*byte_ptr & 0x80)) { // highest bit(63) != 1
    --out_byte;
    for (int i = 0; i < 9; ++i) {
      if (!(*byte_ptr & 0x7F)) {
        --out_byte;
        ul <<= 7;
      } else
        break;
    }
  }

  if (out_byte == 0)
    return new char{0};
  
  char* ret = new char[out_byte], *cpy_ptr = ret;  // store in big-endian
  int i = 0;
  if (out_byte == kMax_uint64OutBytes) {
    *cpy_ptr++ = 0x81;
    ++i;
  }

  while (i++ < out_byte - 1) {
    *cpy_ptr++ = *byte_ptr | 0x80;
    ul <<= 7;
  }
  *cpy_ptr = *byte_ptr & 0x7F;

  return ret;
}

uint64_t VariableInteger::Decode_uint64(const char* src) {
  uint64_t ret = 0;
  const char* byte_ptr = src;
  char* cpy_ptr = reinterpret_cast<char*>(&ret);
  int total_byte = 1;
  
  while (*byte_ptr++ & 0x80) ++total_byte;

  byte_ptr = src;
  *cpy_ptr |= (0x7F & *byte_ptr++);
  while (--total_byte) {
    ret <<= 7;
    *cpy_ptr |= (0x7F & *byte_ptr++);
  }

  return ret;
}
