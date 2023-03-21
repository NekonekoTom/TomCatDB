#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <string>
#include "base.h"

// The class Sequence manages a const char sequence
// by recording size and the reference to the start address.
// A Sequence object DOES NOT hold any actual resources.
class Sequence final {
 public:
  Sequence() = default;
  Sequence(const std::string& str) : data_(str.c_str()), size_(str.size()) {}
  Sequence(const char* data, const uint64_t size) : data_(data), size_(size) {}

  // Allow construction and assignment by copying
  Sequence(const Sequence&) = default;
  Sequence& operator=(const Sequence& obj) = default;

  // Will NOT free the resources referred by data_
  ~Sequence() = default;

  void SkipPrefix(const uint64_t prefix_size) {
    data_ += prefix_size;
    size_ -= prefix_size;
  }

  inline const char* data() const { return data_; }
  inline const uint64_t size() const { return size_; }

 private:
  const char* data_ = "";
  uint64_t size_ = 0;
};

#endif