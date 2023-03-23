#ifndef CACHE_H_
#define CACHE_H_

#include <list>
#include <unordered_map>
#include "base.h"
#include "sequence.h"

class Cache {
 public:
  const uint32_t kDefaultMaxCacheSize = 512;

  Cache() : Cache(512) {}
  explicit Cache(const uint32_t max_size) : size_(0), max_size_(max_size) {}

  Cache(const Cache&) = delete;
  Cache& operator()(const Cache&) = delete;

  virtual ~Cache() = default;

  virtual Sequence Get(const Sequence&) = 0;

  virtual void TryInsert(const Sequence&) = 0;

  virtual uint32_t size() const { return size_; }

 private:
  uint32_t size_;
  uint32_t max_size_;
};

class LRUCache : public Cache {
 public:
  LRUCache();
  ~LRUCache();

  Sequence Get(const Sequence&) { return Sequence(); }

  void TryInsert(const Sequence&) {}

 private:
  std::list<Sequence> data_;
  // std::unordered_map<Sequence, typename std::list<Sequence>::iterator> map_;
};

#endif