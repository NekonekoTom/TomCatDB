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

class SeqHash {
 public:
  uint64_t operator()(const Sequence& seq) const {
    // Use the same hash strategy as std::string.
    // The std::string calls:
    //   std::_Hash_impl::hash(__s.data(), __s.length())
    // (template specialization for std::hash<T>) to get a hash value.
    return std::_Hash_impl::hash(seq.data(), seq.size());
  }
};

class SeqEqual {
 public:
  bool operator()(const Sequence& seq_x, const Sequence& seq_y) const {
    if (seq_x.size() == seq_y.size()) {
      auto len = seq_x.size();
      const char* x = seq_x.data();
      const char* y = seq_y.data();
      while (len-- != 0) {
        if (*x != *y) {
          return false;
        }
      }
      return true;
    }
    return false;
  }
};

#endif