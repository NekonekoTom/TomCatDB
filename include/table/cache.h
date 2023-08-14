#ifndef CACHE_H_
#define CACHE_H_

#include <unordered_map>
#include "base.h"
#include "dual_list.h"
#include "internal_entry.h"
#include "mem_allocator.h"
#include "sequence.h"

template <typename K, typename V>
class Cache {
 public:
  static constexpr uint32_t kDefaultMaxCacheSize = 1 << 9;  // 512

  Cache() : Cache(kDefaultMaxCacheSize) {}
  explicit Cache(const uint32_t capacity) : size_(0), capacity_(capacity) {}

  Cache(const Cache&) = delete;
  Cache& operator()(const Cache&) = delete;

  virtual ~Cache() = default;

  // Try to get a value corresponding to the key. Return true if the key is in
  // the cache, and return the value by reference. Otherwise, return false,
  // and the value is invalid.
  virtual bool Get(const K& key, V& value) = 0;

  virtual void Insert(const K& key, const V& value) = 0;

  virtual uint32_t size() const { return size_; }

  virtual uint32_t capacity() const { return capacity_; }

 protected:
  uint32_t size_;

 private:
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

  ~LRUCache() = default;

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
  } else {
    ++this->size_;
  }
}

// Specialization for LRUCache
// If the key and value are both of type Sequence, the LRUCache builds a copy
// of the key and the value in the cache_pool_.
template <>
class LRUCache<Sequence, Sequence, SeqHash, SeqEqual>
    : public Cache<Sequence, Sequence> {
 public:
  LRUCache() : LRUCache(this->kDefaultMaxCacheSize) {}
  explicit LRUCache(const uint32_t capacity);

  LRUCache(const LRUCache&) = delete;
  LRUCache& operator()(const LRUCache&) = delete;

  ~LRUCache() = default;

  bool Get(const Sequence&, Sequence&);

  // Insert key-value pair into the LRUCache.
  // Note: the "value" is encoded with OpType and the actual value.
  // The first byte of the "value" denotes OpType, and the value data starts
  // from the second byte. The definition of OpType is in internal_entry.h
  // Insertion behavior:
  //   OpType = 1(0x01), and the value data starts from the next byte;
  // Deletion behavior:
  //   OpType = 0(0x00), and the value data is empty.
  void Insert(const Sequence& key, const Sequence& value);

 private:
  struct NodeData {
    Sequence key;
    Sequence val;
    uint32_t key_block_id;
    uint32_t val_block_id;
  };

  using DualList = neko_base::DualList<NodeData>;

  DualList data_;
  std::unordered_map<Sequence, typename DualList::DualListNode*, SeqHash,
                     SeqEqual>
      index_;

  std::shared_ptr<CacheAllocator> cache_pool_;
};

inline LRUCache<Sequence, Sequence, SeqHash, SeqEqual>::LRUCache(
    const uint32_t capacity)
    : Cache<Sequence, Sequence>(capacity) {
  index_ =
      std::unordered_map<Sequence, typename DualList::DualListNode*, SeqHash,
                         SeqEqual>(this->size(), SeqHash(), SeqEqual());

  cache_pool_ = std::make_shared<CacheAllocator>();
}

// The same as the unspecialized version.
inline bool LRUCache<Sequence, Sequence, SeqHash, SeqEqual>::Get(
    const Sequence& key, Sequence& ret_value) {
  if (index_.find(key) != index_.end()) {
    data_.MoveFront(index_[key]);
    ret_value = index_[key]->data.val;
    return true;
  } else {
    // The key is not in the Cache
    return false;
  }
}

// Different from the unspecialized version:
//   Allocate space for the key and the value before insertion, respectively;
//   Unref the space from the cache_pool_ after updating (unref the old value)
//   or eviction (unref the key and the value).
inline void LRUCache<Sequence, Sequence, SeqHash, SeqEqual>::Insert(
    const Sequence& key, const Sequence& value) {
  if (index_.find(key) != index_.end()) {
    // Allocate memory for value only
    uint32_t block_id;
    char* cache_val = cache_pool_->Allocate(value.size(), block_id);
    std::memcpy(cache_val, value.data(), value.size());

    // Unref the old value from the memory pool
    uint32_t old_val_blk_id = index_[key]->data.val_block_id;
    cache_pool_->Unref(old_val_blk_id);

    // Update cache value
    index_[key]->data.val = Sequence(cache_val, value.size());
    index_[key]->data.val_block_id = block_id;

    data_.MoveFront(index_[key]);
  } else {
    // Allocate memory for key and value
    uint32_t k_block_id, v_block_id;
    char* cache_k = cache_pool_->Allocate(key.size(), k_block_id);
    char* cache_v = cache_pool_->Allocate(value.size(), v_block_id);
    std::memcpy(cache_k, key.data(), key.size());
    std::memcpy(cache_v, value.data(), value.size());

    // Insert new record
    NodeData nd{Sequence(cache_k, key.size()), Sequence(cache_v, value.size()),
                k_block_id, v_block_id};
    index_[key] = data_.PushFront(nd);

    ++this->size_;
  }

  if (data_.size() > this->capacity()) {
    auto node_data = data_.Back();

    // Evict from hash map
    index_.erase(node_data.key);

    // Evict from memory pool (unref only, but not delete at once)
    cache_pool_->Unref(node_data.key_block_id);
    cache_pool_->Unref(node_data.val_block_id);

    // Evict from the list
    data_.PopBack();

    --this->size_;
  }
}

class TCCache {
 public:
  static constexpr int kDefaultCacheSize = 1 << 9;

  TCCache() : TCCache(kDefaultCacheSize) {}
  explicit TCCache(const int cache_size);

  TCCache(const TCCache&) = delete;
  TCCache& operator=(const TCCache&) = delete;

  ~TCCache() = default;

  // Insert a key-value pair into the cache. Call InsertOrUpdate().
  bool Insert(const Sequence& key, const Sequence& value);

  // Delete a key-value pair from the cache. Call InsertOrUpdate().
  bool Delete(const Sequence& key);

  // Return true if the key is in the cache, false otherwise.
  // If the key exists, the corresponding value will be returned by reference.
  // The value is returned in std::string instead of Sequence to avoid
  // Sequence's data pointer expiration.
  bool Get(const Sequence& key, std::string& value);

  const uint32_t size() const { return cache_->size(); }

 private:
  // Insert or update the key-value pair in the cache. The value starts with
  // 1 byte denoting the OpType.
  // Insertion behavior:
  //   OpType = 1(0x01), and the value data starts from the next byte;
  // Deletion behavior:
  //   OpType = 0(0x00), and the value data is empty.
  bool InsertOrUpdate(const Sequence& key, const Sequence& value);

  std::shared_ptr<Cache<Sequence, Sequence>> cache_;
};

struct SSTPage {
  uint64_t sst_id;
};

class TCPageCache {
 public:
  static constexpr int kDefaultCacheSize = 1 << 7;

  TCPageCache() : TCPageCache(kDefaultCacheSize) {}
  explicit TCPageCache(const int cache_size);

  TCPageCache(const TCPageCache&) = delete;
  TCPageCache& operator=(const TCPageCache&) = delete;

  ~TCPageCache() = default;

  // Insert an SST page into the cache. Call InsertOrUpdate().
  bool Insert(const SSTPage& page);

  // Delete an SST page from the cache. Call InsertOrUpdate().
  bool Delete(const SSTPage& page);

  // Return true if the page is in the cache, false otherwise.
  bool Get(const SSTPage& page);

  const uint32_t size() const { return cache_->size(); }

 private:
  // Insert or update the key-value pair in the cache.
  bool InsertOrUpdate(const SSTPage& page);

  std::shared_ptr<Cache<uint64_t, std::vector<Sequence>>> cache_;
};

#endif