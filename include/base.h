// Basic macro definition

#ifndef NEKO_BASE_H_
#define NEKO_BASE_H_

#include <cstring> // For std::memcpy
#include <stdint.h> // For uint_t
#include <cassert> // For assert
#include <vector>
#include <mutex> // For concurrency control

// typedef unsigned long U_LONG;
using U_LONG = unsigned long;

namespace neko_base {

inline void memcpy(const char* src, char* des, U_LONG size) {
  // TODO: mutex
  while (size--) *des++ = *src++;
}

}  // namespace neko_base

#endif