#ifndef INTERNAL_ENTRY_H_
#define INTERNAL_ENTRY_H_

#include "sequence.h"
#include "status.h"
#include "varint.h"

// InternalEntry wraps raw key and value Sequence in continuous memory.
// There are no two same InternalEntries in one database instance, so
// an InternalEntry can be used to compare two key-value pairs.
// The format of InternalEntry is as follows:
// +---------------------------------------------------------------+
// |  Key Sequence  |  ID(uint64_t)  |  OpType  |  Value Sequence  |
// +---------------------------------------------------------------+
// OpType denotes current operation type, either 1 for insert or 0 for delete.
class InternalEntry {
 public:
  enum OpType {
    kDelete = 0,
    kInsert = 1
  };

  ~InternalEntry() = default;

  // Encode given key and value to an InternalEntry
  static Status EncodeInternal(const Sequence& key, const Sequence& value,
                               const uint64_t id, const OpType op_type,
                               char* internal_entry);

  // Retuen OpType of the given InternalEntry
  static OpType EntryOpType(const char* internal_entry);

  // Retuen the key of the given InternalEntry by Sequence
  static Sequence* EntryKey(const char* internal_entry);

  // Retuen the key of the given InternalEntry by Sequence
  static Sequence* EntryValue(const char* internal_entry);

 private:
  InternalEntry() = delete;
  InternalEntry(const InternalEntry&) = delete;
  InternalEntry& operator=(const InternalEntry&) = delete;
};

#endif