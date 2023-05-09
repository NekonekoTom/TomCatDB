#ifndef CACHE_H_
#define CACHE_H_

#include <unordered_map>
#include "base.h"
#include "dual_list.h"
#include "sequence.h"

template <typename K, typename V>
class Cache {
 public:
  const uint32_t kDefaultMaxCacheSize = 1 << 9;  // 512

  Cache() : Cache(kDefaultMaxCacheSize) {}
  explicit Cache(const uint32_t capacity) : size_(0), capacity_(capacity) {}

  Cache(const Cache&) = delete;
  Cache& operator()(const Cache&) = delete;

  virtual ~Cache() = default;

  virtual bool Get(const K&, V&) = 0;

  virtual void Insert(const K&, const V&) = 0;

  virtual uint32_t size() const { return size_; }

  virtual uint32_t capacity() const { return capacity_; }

 private:
  uint32_t size_;
  uint32_t capacity_;
};

template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>>
class LRUCache : public Cache<K, V> {
 public:
  using DualList = neko_base::DualList<std::pair<K, V>>;

  LRUCache() : LRUCache(this->kDefaultMaxCacheSize) {}
  explicit LRUCache(const uint32_t capacity);

  LRUCache(const LRUCache&) = delete;
  LRUCache& operator()(const LRUCache&) = delete;

  ~LRUCache();

  bool Get(const K&, V&);

  void Insert(const K&, const V&);

 private:
  DualList data_;
  std::unordered_map<K, typename DualList::DualListNode*, Hash, Equal> index_;
};

template <typename K, typename V, typename Hash, typename Equal>
LRUCache<K, V, Hash, Equal>::LRUCache(const uint32_t capacity)
    : Cache<K, V>(capacity) {
  index_ = std::unordered_map<K, typename DualList::DualListNode*, Hash, Equal>(
      this->size(), Hash(), Equal());
}

template <typename K, typename V, typename Hash, typename Equal>
LRUCache<K, V, Hash, Equal>::~LRUCache() {}

template <typename K, typename V, typename Hash, typename Equal>
bool LRUCache<K, V, Hash, Equal>::Get(const K& key, V& ret_value) {
  if (index_.find(key) != index_.end()) {
    data_.MoveFront(index_[key]);
    ret_value = index_[key]->data.second;
    return true;
  } else {
    // The key is not in the Cache
    return false;
  }
}

template <typename K, typename V, typename Hash, typename Equal>
void LRUCache<K, V, Hash, Equal>::Insert(const K& key, const V& value) {
  if (index_.find(key) != index_.end()) {
    // Update cache value
    index_[key]->data.second = value;
    data_.MoveFront(index_[key]);
  } else {
    // Insert new record
    index_[key] = data_.PushFront(std::make_pair(key, value));
  }

  if (data_.size() > this->capacity()) {
    // Evict from hash map
    index_.erase(data_.Back().first);

    // Evict from the list
    data_.PopBack();
  }
}

#endif