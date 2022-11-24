#include "comparator.h"

bool SequenceComparator::GreaterOrEquals(const Sequence& first,
                                        const Sequence& second) const {
  auto first_size = first.size();
  auto second_size = second.size();

  const char* first_data = first.data();
  const char* second_data = second.data();

  while (first_size > 0 && second_size > 0 && *first_data == *second_data) {
    --first_size;
    --second_size;
    ++first_data;
    ++second_data;
  }

  if (first_size <= 0 && second_size <= 0 || second_size <= 0) {
    return true;
  } else if (first_size > 0) {
    return *first_data >= *second_data;
  }

  return false;
}

bool SequenceComparator::LessOrEquals(const Sequence& first,
                                     const Sequence& second) const {
  auto first_size = first.size();
  auto second_size = second.size();

  const char* first_data = first.data();
  const char* second_data = second.data();

  while (first_size > 0 && second_size > 0 && *first_data == *second_data) {
    --first_size;
    --second_size;
    ++first_data;
    ++second_data;
  }

  if (first_size <= 0 && second_size <= 0 || first_size <= 0) {
    return true;
  } else if (second_size > 0) {
    return *first_data <= *second_data;
  }

  return false;
}

bool SequenceComparator::Greater(const Sequence& first,
                                 const Sequence& second) const {
  return !LessOrEquals(first, second);
}

bool SequenceComparator::Less(const Sequence& first,
                              const Sequence& second) const {
  return !GreaterOrEquals(first, second);
}

bool SequenceComparator::Equal(const Sequence& first,
                               const Sequence& second) const {
  auto first_size = first.size();
  auto second_size = second.size();

  const char* first_data = first.data();
  const char* second_data = second.data();

  while (first_size > 0 && second_size > 0 && *first_data == *second_data) {
    --first_size;
    --second_size;
    ++first_data;
    ++second_data;
  }

  return (first_size == second_size) && (first_size == 0);
}

bool InternalEntryComparator::GreaterOrEquals(const char* const& first,
                                             const char* const& second) const {
  if (*first == 0) return true;
  if (*second == 0) return false;

  auto first_key_size = coding::DecodeVarint64(first);
  auto second_key_size = coding::DecodeVarint64(second);

  const char* first_key_data = first + coding::SizeOfVarint(first);
  const char* second_key_data = second + coding::SizeOfVarint(second);

  while (first_key_size > 0 && second_key_size > 0 && *first_key_data == *second_key_data) {
    --first_key_size;
    --second_key_size;
    ++first_key_data;
    ++second_key_data;
  }

  // if (first_key_size <= 0 && second_key_size <= 0) {
  //   // If the first and second param have the same key, then:
  //   // Condition 1: OpType(first) > OpType(second)
  //   // Condition 2: ID(first) >= ID(second)
  //   return *first_key_data > *second_key_data ||
  //          *reinterpret_cast<const uint64_t*>(first_key_data + 1) >= 
  //              *reinterpret_cast<const uint64_t*>(second_key_data + 1);
  // } else if (second_key_size <= 0) {
  //   return true;
  // } else if (first_key_size > 0) {
  //   return *first_key_data >= *second_key_data;
  // }

  if (first_key_size <= 0 && second_key_size <= 0) {
    // If the first and second param have the same key, then:
    // Condition: ID(first) >= ID(second)
    return *reinterpret_cast<const uint64_t*>(first_key_data) >= 
               *reinterpret_cast<const uint64_t*>(second_key_data);
  } else if (second_key_size <= 0) {
    return true;
  } else if (first_key_size > 0) {
    return *first_key_data >= *second_key_data;
  }

  return false;
}

bool InternalEntryComparator::LessOrEquals(const char* const& first,
                                          const char* const& second) const {
  if (*first == 0) return true;
  if (*second == 0) return false;

  auto first_key_size = coding::DecodeVarint64(first);
  auto second_key_size = coding::DecodeVarint64(second);

  const char* first_key_data = first + coding::SizeOfVarint(first);
  const char* second_key_data = second + coding::SizeOfVarint(second);

  while (first_key_size > 0 && second_key_size > 0 && *first_key_data == *second_key_data) {
    --first_key_size;
    --second_key_size;
    ++first_key_data;
    ++second_key_data;
  }

  // if (first_key_size <= 0 && second_key_size <= 0) {
  //   // If the first and second param have the same key, then:
  //   // Condition 1: OpType(first) < OpType(second)
  //   // Condition 2: ID(first) <= ID(second)
  //   return *first_key_data < *second_key_data ||
  //          *reinterpret_cast<const uint64_t*>(first_key_data + 1) <=
  //              *reinterpret_cast<const uint64_t*>(second_key_data + 1);
  // } else if (first_key_size <= 0) {
  //   return true;
  // } else if (second_key_size > 0) {
  //   return *first_key_data <= *second_key_data;
  // }

  if (first_key_size <= 0 && second_key_size <= 0) {
    // If the first and second param have the same key, then:
    // Condition: ID(first) <= ID(second)
    return *reinterpret_cast<const uint64_t*>(first_key_data) <=
               *reinterpret_cast<const uint64_t*>(second_key_data);
  } else if (first_key_size <= 0) {
    return true;
  } else if (second_key_size > 0) {
    return *first_key_data <= *second_key_data;
  }

  return false;
}

bool InternalEntryComparator::Greater(const char* const& first,
                                      const char* const& second) const {
  return !LessOrEquals(first, second);
}

bool InternalEntryComparator::Less(const char* const& first,
                                   const char* const& second) const {
  return !GreaterOrEquals(first, second);
}

bool InternalEntryComparator::Equal(const char* const& first,
                                    const char* const& second) const {
  if (*first == 0 || *second == 0) {
    return *first == *second;
  }

  auto first_key_size = coding::DecodeVarint64(first);
  auto second_key_size = coding::DecodeVarint64(second);

  const char* first_key_data = first + coding::SizeOfVarint(first);
  const char* second_key_data = second + coding::SizeOfVarint(second);

  while (first_key_size > 0 && second_key_size > 0 && *first_key_data == *second_key_data) {
    --first_key_size;
    --second_key_size;
    ++first_key_data;
    ++second_key_data;
  }

  return (first_key_size == second_key_size) && (first_key_size == 0);
}