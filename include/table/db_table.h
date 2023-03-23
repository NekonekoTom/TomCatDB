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
  TCTable() = delete;
  TCTable(RAIILock& lock,
          const std::shared_ptr<InternalEntryComparator>& comparator,
          const uint64_t first_entry_id);

  TCTable(const TCTable&) = delete;
  TCTable& operator=(const TCTable&) = delete;

  ~TCTable();

  const Sequence Get(const Sequence& key) const;

  const Sequence Get(const char* internal_entry) const;

  Status Insert(const Sequence& key, const Sequence& value);

  Status Delete(const Sequence& key);

  // Similar to Get()
  bool ContainsKey(const Sequence& key) const;

  // Return read-only entry set for deserialization
  const std::vector<const char*> EntrySet() const { return table_.EntrySet(); }

  // Return current memory usage of the TCTable
  const uint32_t MemUsage() const { return mem_allocator_->MemUsage(); }

  const uint64_t GetNextEntryID() const { return entry_id_; }

 private:
  // Stores char pointers only, the actual resources are managed by mem_allocator_
  SkipList<const char*, InternalEntryComparator> table_;

  // Responsible for allocating and managing the data resources
  MemAllocator* const mem_allocator_;

  // Responsible for allocating and managing the data resources FOR QUERY
  MemAllocator* const query_allocator_;

  uint64_t entry_id_ = 0;  // TODO: The entry_id_ should be globally unique

  RAIILock& table_lock_;
};

#endif