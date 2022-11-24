#include "db_table.h"

TCTable::TCTable(RAIILock& lock, const int default_size)
    : kDefaultMaxTCTableSize(default_size),
      table_(new InternalEntryComparator,
             new char),  // Waste 1 byte here, will never delete.
      mem_allocator_(new MemAllocator),
      table_lock_(lock) {}

TCTable::~TCTable() {
  if (mem_allocator_ != nullptr)
    delete mem_allocator_;
}

const Sequence* TCTable::Get(const Sequence& key) {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  // The value, ID, and op_type are invalid
  Status enc = InternalEntry::EncodeInternal(
      key, Sequence(), static_cast<uint64_t>(0) - 1, InternalEntry::kDelete,
      internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    // const SkipListNode<const char*>* the_node = table_.Get(internal_entry);
    auto the_node = table_.Get(internal_entry);
    table_lock_.Unlock();

    if (the_node != nullptr) {
      if (InternalEntry::EntryOpType(the_node->key_) == InternalEntry::kInsert) {
        // return new Sequence(the_node->key_, 0);
        return InternalEntry::EntryValue(the_node->key_);
      }
    }
  }

  return nullptr;
}

Status TCTable::Insert(const Sequence& key, const Sequence& value) {
  // See InternalEntry.h for format info
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9 +
                        coding::SizeOfVarint(value.size()) + value.size();

  table_lock_.Lock();
  // TCTable is in charge of allocating and managing the memory
  // The underlying SkipList DOES NOT hold any data resources
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  Status enc = InternalEntry::EncodeInternal(
      key, value, entry_id_++, InternalEntry::OpType::kInsert, internal_entry);
  if (enc.StatusNoError()) {
    table_lock_.Lock();
    table_.Insert(internal_entry);
    table_lock_.Unlock();
  }

  return Status::NoError();
}

Status TCTable::Delete(const Sequence& key) {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  Status enc = InternalEntry::EncodeInternal(key, Sequence(), entry_id_++,
                                             InternalEntry::OpType::kDelete,
                                             internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    table_.Insert(internal_entry);
    table_lock_.Unlock();
  }

  return Status::NoError();
}

bool TCTable::ContainsKey(const Sequence& key) {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  // The value, ID, and op_type are invalid
  Status enc = InternalEntry::EncodeInternal(
      key, Sequence(), static_cast<uint64_t>(0) - 1, InternalEntry::kDelete,
      internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    // const SkipListNode<const char*>* the_node = table_.Get(internal_entry);
    auto the_node = table_.Get(internal_entry);
    table_lock_.Unlock();

    if (the_node != nullptr) {
      if (InternalEntry::EntryOpType(the_node->key_) == InternalEntry::kInsert)
        return true;
    }
  }

  return false;
}