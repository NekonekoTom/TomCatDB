// Basic macro definition

#ifndef NEKO_BASE_H_
#define NEKO_BASE_H_

#include <stdint.h>   // For uint_t
#include <algorithm>  // For std::remove
#include <atomic>     // For std::atomic
#include <cassert>    // For assert
#include <cmath>
#include <cstring>    // For std::memcpy
#include <list>
#include <memory>
#include <mutex>      // For concurrency control
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "status.h"

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