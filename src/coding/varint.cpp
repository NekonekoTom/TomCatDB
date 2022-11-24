#include "varint.h"

namespace coding
{

const uint32_t DecodeVarint32(const char* data_ptr) {
  uint32_t ret = 0;
  char* ret_ptr = reinterpret_cast<char*>(&ret);

  // Byte number: 1
  *ret_ptr = *data_ptr & 0x7F;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= *++data_ptr << 7;

  // Byte number: 2
  *ret_ptr = (*data_ptr & 0x7F) >> 1;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x3) << 6;

  // Byte number: 3
  *ret_ptr = (*data_ptr & 0x7F) >> 2;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x7) << 5;

  // Byte number: 4
  *ret_ptr = (*data_ptr & 0x7F) >> 3;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0xF) << 4;

  // Byte number: 5
  *ret_ptr = (*data_ptr & 0x7F) >> 4;

  return ret;
}

const uint64_t DecodeVarint64(const char* data_ptr) {
  uint64_t ret = 0;
  char* ret_ptr = reinterpret_cast<char*>(&ret);

  // Byte number: 1
  *ret_ptr = *data_ptr & 0x7F;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= *++data_ptr << 7;

  // Byte number: 2
  *ret_ptr = (*data_ptr & 0x7F) >> 1;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x3) << 6;

  // Byte number: 3
  *ret_ptr = (*data_ptr & 0x7F) >> 2;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x7) << 5;

  // Byte number: 4
  *ret_ptr = (*data_ptr & 0x7F) >> 3;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0xF) << 4;

  // Byte number: 5
  *ret_ptr = (*data_ptr & 0x7F) >> 4;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x1F) << 3;

  // Byte number: 6
  *ret_ptr = (*data_ptr & 0x7F) >> 5;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x3F) << 2;

  // Byte number: 7
  *ret_ptr = (*data_ptr & 0x7F) >> 6;
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr++ |= (*++data_ptr & 0x7F) << 1;

  // Byte number: 8
  *ret_ptr = (*++data_ptr & 0x7F);
  if (!(*data_ptr & 0x80)) return ret;
  else *ret_ptr |= 0x80;

  return ret;
}

// Encode uint32_t data into dest.
// Return: the address at the end
char* EncodeVarint32(const uint32_t data, char* dest) {
  char* ret = dest;
  const char* data_ptr = reinterpret_cast<const char*>(&data);
  if (data < 1 << 7) {                          // Encode 1 byte
    *ret = *data_ptr & 0x7F;
  } else if (data < 1 << 14) {                  // Encode 2 byte
    *ret++ = *data_ptr | 0x80;
    *ret = (*data_ptr++ >> 7 & 0x1) | (*data_ptr << 1) & 0x7F;
  } else if (data < 1 << 21) {                  // Encode 3 byte
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1) | (*data_ptr << 1) | 0x80;
    *ret = (*data_ptr++ >> 6 & 0x3) | (*data_ptr << 2) & 0x7F;
  } else if (data < 1 << 28) {                  // Encode 4 byte
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3) | (*data_ptr << 2) | 0x80;
    *ret = (*data_ptr++ >> 5 & 0x7) | (*data_ptr << 3) & 0x7F;
  } else {                                      // Encode 5 byte
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7) | (*data_ptr << 3) | 0x80;
    *ret = (*data_ptr >> 4 & 0xF) & 0x7F;
  }
  return ++ret;
}

char* EncodeVarint64(const uint64_t data, char* dest) {
  char* ret = dest;
  const char* data_ptr = reinterpret_cast<const char*>(&data);
  if (data < 1 << 7) {                          // Encode 1 byte
    *ret = *data_ptr & 0x7F;
  } else if (data < 1 << 14) {                  // Encode 2 bytes
    *ret++ = *data_ptr | 0x80;
    *ret =   (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) & 0x7F;
  } else if (data < 1 << 21) {                  // Encode 3 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret =   (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) & 0x7F;
  } else if (data < 1 << 28) {                  // Encode 4 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret =   (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) & 0x7F;
  } else if (data < 0x800000000) {              // Encode 5 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret =   (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) & 0x7F;
  } else if (data < 0x40000000000) {            // Encode 6 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret++ = (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) | 0x80;
    *ret =   (*data_ptr++ >> 3 & 0x1F) | (*data_ptr << 5) & 0x7F;
  } else if (data < 0x2000000000000) {          // Encode 7 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret++ = (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) | 0x80;
    *ret++ = (*data_ptr++ >> 3 & 0x1F) | (*data_ptr << 5) | 0x80;
    *ret =   (*data_ptr++ >> 2 & 0x3F) | (*data_ptr << 6) & 0x7F;
  } else if (data < 0x100000000000000) {        // Encode 8 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret++ = (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) | 0x80;
    *ret++ = (*data_ptr++ >> 3 & 0x1F) | (*data_ptr << 5) | 0x80;
    *ret++ = (*data_ptr++ >> 2 & 0x3F) | (*data_ptr << 6) | 0x80;
    *ret =   (*data_ptr++ >> 1 & 0x7F) | (*data_ptr << 7) & 0x7F;
  } else if (data <= 0x7FFFFFFFFFFFFFFF) {      // Encode 9 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret++ = (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) | 0x80;
    *ret++ = (*data_ptr++ >> 3 & 0x1F) | (*data_ptr << 5) | 0x80;
    *ret++ = (*data_ptr++ >> 2 & 0x3F) | (*data_ptr << 6) | 0x80;
    *ret++ = (*data_ptr++ >> 1 & 0x7F) | (*data_ptr << 7) | 0x80;
    *ret =   (*data_ptr)                                  & 0x7F;
  } else {                                      // Encode 10 bytes
    *ret++ = *data_ptr | 0x80;
    *ret++ = (*data_ptr++ >> 7 & 0x1 ) | (*data_ptr << 1) | 0x80;
    *ret++ = (*data_ptr++ >> 6 & 0x3 ) | (*data_ptr << 2) | 0x80;
    *ret++ = (*data_ptr++ >> 5 & 0x7 ) | (*data_ptr << 3) | 0x80;
    *ret++ = (*data_ptr++ >> 4 & 0xF ) | (*data_ptr << 4) | 0x80;
    *ret++ = (*data_ptr++ >> 3 & 0x1F) | (*data_ptr << 5) | 0x80;
    *ret++ = (*data_ptr++ >> 2 & 0x3F) | (*data_ptr << 6) | 0x80;
    *ret++ = (*data_ptr++ >> 1 & 0x7F) | (*data_ptr << 7) | 0x80;
    *ret++ = (*data_ptr)                                  | 0x80;
    // *ret =   (*data_ptr >> 7 & 0x1 )                      & 0x7F;
    *ret = 0x1;
  }
  return ++ret;
}

} // namespace coding