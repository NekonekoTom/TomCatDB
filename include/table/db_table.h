#ifndef DB_TABLE_H_
#define DB_TABLE_H_

#include "comparator.h"
#include "internal_entry.h"
#include "mem_allocator.h"
#include "raii_lock.h"
#include "skiplist.h"
#include "status.h"

// In-memory volatile table for fast access to latest data
// The TCTable is guaranteed to be thread-safe
class TCTable {
 public:
  // Max TCTable size. When the capacity reaches the limit,
  // the TCTable will be transferred to an immutable table
  // and written to level 0 sst file for persistence.
  const int kDefaultMaxTCTableSize;

  TCTable() = delete;
  TCTable(RAIILock& lock, const int default_size = 1 << 12);

  TCTable(const TCTable&) = delete;
  TCTable& operator=(const TCTable&) = delete;

  ~TCTable();

  const Sequence Get(const Sequence& key) const;

  Status Insert(const Sequence& key, const Sequence& value);

  Status Delete(const Sequence& key);

  // Similar to Get()
  bool ContainsKey(const Sequence& key) const;

  // Return read-only entry set for deserialization
  const std::vector<const char*> EntrySet() const { return table_.EntrySet(); }

 private:
  // Stores char pointers only, the actual resources are managed by mem_allocator_
  SkipList<const char*, InternalEntryComparator> table_;

  // Responsible for allocating and managing the data resources
  MemAllocator* const mem_allocator_;

  uint64_t entry_id_ = 0;

  RAIILock& table_lock_;
};

#endif