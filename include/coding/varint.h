#ifndef VARINT_H_
#define VARINT_H_

#include "base.h"

namespace coding
{

// All variable integers are stored in little endian
// The coding rules conformed with the Protocol Buffers

// Decode varint32 from a char sequence
const uint32_t DecodeVarint32(const char* data_ptr);

// Decode varint64 from a char sequence
const uint64_t DecodeVarint64(const char* data_ptr);

// Encode uint32_t into a char sequence. The space MUST have been allocated.
// Returns the next byte to the end.
char* EncodeVarint32(const uint32_t data, char* dest);

// Encode uint64_t into a char sequence. The space MUST have been allocated.
// Returns the next byte to the end.
char* EncodeVarint64(const uint64_t data, char* dest);

// Calculate the size of varint32 or varint64 by char*
inline const uint32_t SizeOfVarint(const char* data_ptr);

// Calculate the size of varint32 or varint64 by uint64_t
inline const uint32_t SizeOfVarint(uint64_t data);

inline const uint32_t SizeOfVarint(const char* data_ptr) {
  uint32_t ret = 1;
  while (*data_ptr++ & 0x80) ++ret;
  return ret;
}

inline const uint32_t SizeOfVarint(uint64_t data) {
  if (data & 0x8000000000000000) return 10;

  uint64_t non_const_data = data;
  uint32_t ret = 1;
  while ((non_const_data >>= 7) != 0) ++ret;
  
  return ret;
}

} // namespace coding


#endif