#include "cache.h"

TCCache::TCCache(const int cache_size) {
  cache_ = std::make_shared<LRUCache<Sequence, Sequence, SeqHash, SeqEqual>>(
      cache_size);
}

bool TCCache::Insert(const Sequence& key, const Sequence& value) {
  char* tmp = new char[value.size() + 1];
  *tmp = InternalEntry::OpType::kInsert;
  std::memcpy(tmp + 1, value.data(), value.size());

  bool ret = InsertOrUpdate(key, Sequence(tmp, value.size() + 1));
  delete[] tmp;

  return ret;
}

bool TCCache::Delete(const Sequence& key) {
  char tmp{InternalEntry::OpType::kDelete};

  return InsertOrUpdate(key, Sequence(&tmp, 1));
}

bool TCCache::Get(const Sequence& key, std::string& value) {
  Sequence tmp;

  if (cache_->Get(key, tmp)) {
    switch (*tmp.data()) {
      case InternalEntry::OpType::kDelete:
        return false;
        break;
      case InternalEntry::OpType::kInsert:
        // strip the first byte
        value = std::string(tmp.data() + 1, tmp.size() - 1);
        return true;
      default:
        return false;
    }
  }

  return false;
}

bool TCCache::InsertOrUpdate(const Sequence& key, const Sequence& value) {
  cache_->Insert(key, value);

  return true;
}