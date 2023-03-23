#ifndef COMPARATOR_H_
#define COMPARATOR_H_

#include "internal_entry.h"
#include "sequence.h"

// Comparator for comparing Sequence objects
template <typename T>
class Comparator {
 public:
  Comparator() = default;
  Comparator(const Comparator&) = default;
  Comparator& operator=(const Comparator&) = delete;

  virtual ~Comparator() = default;

  // Return true if first param >= second param
  virtual bool GreaterOrEquals(const T&, const T&) const = 0;

  // Return true if first param <= second param
  virtual bool LessOrEquals(const T&, const T&) const = 0;

  // Return true if first param > second param
  virtual bool Greater(const T&, const T&) const = 0;

  // Return true if first param < second param
  virtual bool Less(const T&, const T&) const = 0;

  // Return true if first param == second param
  virtual bool Equal(const T&, const T&) const = 0;

 private:
  // TODO: Any data members? Use static functions instead?
};

// Comparator for comparing Sequence objects
class SequenceComparator : public Comparator<Sequence> {
 public:
  SequenceComparator() = default;
  SequenceComparator(const SequenceComparator&) = delete;
  SequenceComparator& operator=(const SequenceComparator&) = delete;

  ~SequenceComparator() = default;

  bool GreaterOrEquals(const Sequence&, const Sequence&) const;
  bool LessOrEquals(const Sequence&, const Sequence&) const;
  bool Greater(const Sequence&, const Sequence&) const;
  bool Less(const Sequence&, const Sequence&) const;
  bool Equal(const Sequence&, const Sequence&) const;

 private:
  // TODO: Any data members? Use static functions instead?
};

class IntegerComparator : public Comparator<int> {
 public:
  IntegerComparator() = default;
  IntegerComparator(const IntegerComparator&) = default;
  IntegerComparator& operator=(const IntegerComparator&) = delete;

  ~IntegerComparator() = default;

  bool GreaterOrEquals(const int& x, const int& y) const { return x >= y; }
  bool LessOrEquals(const int& x, const int& y) const { return x <= y; }
  bool Greater(const int& x, const int& y) const { return x > y; }
  bool Less(const int& x, const int& y) const { return x < y; }
  bool Equal(const int& x, const int& y) const { return x == y; }

 private:
  // TODO: Any data members? Use static functions instead?
};

// For an InternalEntry, the first byte in char sequence must NOT be 0
// since a 0-start sequence is treated as an invalid tag (infinite).
class InternalEntryComparator : public Comparator<const char*> {
 public:
  InternalEntryComparator() = default;
  InternalEntryComparator(const InternalEntryComparator&) = default;
  InternalEntryComparator& operator=(const InternalEntryComparator&) = delete;

  ~InternalEntryComparator() = default;

  // Return true if the first parameter has a larger key AND a larger id.
  // If the two parameters have the same key, return true if id(x) > id (y).
  // For comparing only the key part, Equal() should be called.
  bool GreaterOrEquals(const char* const&, const char* const&) const;

  // See GreaterOrEquals() for details.
  bool LessOrEquals(const char* const&, const char* const&) const;

  // Call !LessOrEquals()
  bool Greater(const char* const&, const char* const&) const;

  // Call !GreaterOrEquals()
  bool Less(const char* const&, const char* const&) const;

  // Compare the KEY part only. If key(x) == key(y), return true.
  // The semantics of this function are not the same as GreaterOrEquals() or
  // LessOrEquals().
  bool Equal(const char* const&, const char* const&) const;

  bool GreaterOrEquals(const std::string& x, const std::string& y) const {
    return GreaterOrEquals(x.c_str(), y.c_str());
  }
  bool LessOrEquals(const std::string& x, const std::string& y) const {
    return LessOrEquals(x.c_str(), y.c_str());
  }
  bool Greater(const std::string& x, const std::string& y) const {
    return Greater(x.c_str(), y.c_str());
  }
  bool Less(const std::string& x, const std::string& y) const {
    return Less(x.c_str(), y.c_str());
  }
  bool Equal(const std::string& x, const std::string& y) const {
    return Equal(x.c_str(), y.c_str());
  }

  // Functor for std::priority_queue
  bool operator()(const Sequence& x, const Sequence& y) {
    return Greater(x.data(), y.data());
  }

 private:
  // TODO: Any data members? Use static functions instead?
};

class MergeComparator : public InternalEntryComparator {
 public:
  // Functor for std::priority_queue
  bool operator()(const std::pair<Sequence, int>& x,
                  const std::pair<Sequence, int>& y) {
    return Greater(x.first.data(), y.first.data());
  }

  // Functor for std::priority_queue
  bool operator()(const std::tuple<Sequence, int, int>& x,
                  const std::tuple<Sequence, int, int>& y) {
    return Greater(std::get<0>(x).data(), std::get<0>(y).data());
  }
};

class QueryComparator : public InternalEntryComparator {
  public:
  QueryComparator() = default;
  QueryComparator(const QueryComparator&) = default;
  QueryComparator& operator=(const QueryComparator&) = delete;

  ~QueryComparator() = default;

  // Return true if the first parameter has a larger key.
  bool GreaterOrEquals(const char* const&, const char* const&) const;

  // Return true if the first parameter has a smaller key.
  bool LessOrEquals(const char* const&, const char* const&) const;

  // // Call !LessOrEquals()
  // bool Greater(const char* const&, const char* const&) const;

  // // Call !GreaterOrEquals()
  // bool Less(const char* const&, const char* const&) const;

  // // Compare the KEY part only. If key(x) == key(y), return true.
  // // The semantics of this function are consistent with GreaterOrEquals() and
  // // LessOrEquals().
  // bool Equal(const char* const&, const char* const&) const;

  // Functor for std::priority_queue
  bool operator()(const Sequence& x, const Sequence& y) {
    return Greater(x.data(), y.data());
  }
};

#endif