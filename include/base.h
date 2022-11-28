// Basic macro definition

#ifndef NEKO_BASE_H_
#define NEKO_BASE_H_

#include <stdint.h>  // For uint_t
#include <cassert>   // For assert
#include <cstring>   // For std::memcpy
#include <mutex>     // For concurrency control
#include <vector>
#include <memory>

// typedef unsigned long U_LONG;
using U_LONG = unsigned long;

namespace neko_base {

// // Deprecated
// inline void memcpy(const char* src, char* des, U_LONG size) {
//   // TODO: mutex
//   while (size--) *des++ = *src++;
// }

}  // namespace neko_base

#endif