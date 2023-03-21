#ifndef VARIABLE_INT_H_
#define VARIABLE_INT_H_

#include "base.h"

class VariableInteger {
 public:
  static const int kMax_uint64OutBytes = 10;

  // Deprecated: Not compatible with Protobuf definitions.
  // Encode unsigned long to char*.
  // e.g. | 1 0 0 0 0 0 0 0 | where tag = 1, data = "0000000";
  // If the tag bit equals to 0, it means the following byte of data is unvalid.
  static char* Encode_uint64(const uint64_t);

  // Deprecated: Not compatible with Protobuf definitions.
  // Decode char* to unsigned long.
  static uint64_t Decode_uint64(const char*);
  static char* ProtobufEncode64(const uint64_t);
  static inline int Encode_uint64Size(const uint64_t);

 private:
  VariableInteger(const VariableInteger&) = delete;
  VariableInteger() = delete;
  ~VariableInteger();
};

inline int VariableInteger::Encode_uint64Size(const uint64_t src) {
  if (src <= 0x7F)
    return 1;
  else if (src <= 0x3FFF)
    return 2;
  else if (src <= 0x1FFFFF)
    return 3;
  else if (src <= 0xFFFFFFF)
    return 4;
  else if (src <= 0x7FFFFFFFF)
    return 5;
  else if (src <= 0x3FFFFFFFFFF)
    return 6;
  else if (src <= 0x1FFFFFFFFFFFF)
    return 7;
  else if (src <= 0xFFFFFFFFFFFFFF)
    return 8;
  else if (src <= 0x7FFFFFFFFFFFFFFF)
    return 9;
  else
    return 10;
}

#endif