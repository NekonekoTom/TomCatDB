#include "internal_entry.h"

Status InternalEntry::EncodeInternal(const Sequence& key, const Sequence& value,
                                     const uint64_t id, const OpType op_type,
                                     char* internal_entry) {
  char* intnl_ptr = internal_entry;
  // uint64_t k_size = coding::SizeOfVarint(key.size()) + key.size();
  // uint64_t v_size = coding::SizeOfVarint(value.size()) + value.size();

  intnl_ptr = coding::EncodeVarint64(key.size(), intnl_ptr);

  memcpy(intnl_ptr, key.data(), key.size());
  intnl_ptr += key.size();

  *reinterpret_cast<uint64_t*>(intnl_ptr) = id;
  intnl_ptr += 8;

  // Set OpType = 1(Insert) or 0(Delete)
  *intnl_ptr++ = op_type;

  if (op_type == kInsert) {
    intnl_ptr = coding::EncodeVarint64(value.size(), intnl_ptr);
    memcpy(intnl_ptr, value.data(), value.size());
  }

  return Status();
}

InternalEntry::OpType InternalEntry::EntryOpType(const char* internal_entry) {
  uint64_t key_size = coding::DecodeVarint64(internal_entry);
  internal_entry += coding::SizeOfVarint(key_size) + key_size + 8;

  assert(*internal_entry == 0 || *internal_entry == 1);

  switch (*internal_entry) {
    case 0:
      return kDelete;
    case 1:
      return kInsert;
  }

  // Should never reach
  return kDelete;
}

Sequence* InternalEntry::EntryKey(const char* internal_entry) {
  uint64_t size = coding::DecodeVarint64(internal_entry);

  return new Sequence(internal_entry + coding::SizeOfVarint(internal_entry),
                      size);
}

Sequence* InternalEntry::EntryValue(const char* internal_entry) {
  uint64_t size = coding::DecodeVarint64(internal_entry);
  internal_entry += coding::SizeOfVarint(size) + size + 9;

  return new Sequence(internal_entry + coding::SizeOfVarint(internal_entry),
                      coding::DecodeVarint64(internal_entry));
}