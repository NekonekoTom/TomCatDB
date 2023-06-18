#ifndef MEM_POOL_H_
#define MEM_POOL_H_

#include <vector>
#include "base.h"

class MemAllocator {
 public:
  // Default block size 4096B
  const uint64_t kDefaultBlockSize = 1 << 12;

  // When requesting a block of size > kMaxSectorSize, to avoid producing too much
  // internal fragmentation, a new full block will be allocated.
  const uint64_t kMaxSectorSize;

  MemAllocator() : kMaxSectorSize(kDefaultBlockSize / 2) {
    InitAllocator();
  }

  explicit MemAllocator(const uint64_t default_block_size)
      : kDefaultBlockSize(default_block_size),
        kMaxSectorSize(kDefaultBlockSize / 2) {
    InitAllocator();
  }

  MemAllocator(const MemAllocator&) = delete;
  MemAllocator& operator=(const MemAllocator&) = delete;

  virtual ~MemAllocator();

  virtual char* Allocate(const uint64_t size);
  char* AllocateNewBlock(const uint64_t size);
  char* AllocateFullBlock(const uint64_t size);

  // Deprecate the old data in block[block_id] and reallocate a new block
  virtual char* Reallocate(const uint64_t size, const int block_id);

  void Ref(const uint32_t block_id);

  const int Unref(const uint32_t block_id);

  void RefLast(const int times) { ref.back() += times; }

  void RefBlock(const int times, const uint32_t block_id) {
    ref[block_id] += times;
  }

  const uint32_t MemUsage() const {
    return block_ptr_.size() * kDefaultBlockSize - remaining_size_;
  }

  const uint32_t BlockCount() const { return block_ptr_.size(); }

  // Release idle space in the memory pool.
  // Those blocks whose ref is 0 will be deleted.
  void ReleaseIdleSpace();

  void ReleaseIdleSpace(const int block_id);

 protected:
  uint64_t remaining_size_ = kDefaultBlockSize;
  char* start_addr_ = nullptr;

  std::vector<char*> block_ptr_;
  std::vector<int> ref;

 private:
  void InitAllocator() {
    start_addr_ = AllocateFullBlock(kDefaultBlockSize);
    Unref(0);  // When allocating the first block, ref[0] will be increased but
               // not actually reffed by any data.
  }
};

class MergeAllocator : public MemAllocator {
 public:
  MergeAllocator() : MemAllocator(0) {}

  MergeAllocator(const MergeAllocator&) = delete;
  MergeAllocator& operator=(const MergeAllocator&) = delete;

  virtual ~MergeAllocator() {}

  char* Allocate(const uint64_t size) {
    // There is a one to one correspondence between a memory block and a DataBlock
    return MemAllocator::Allocate(size);
  }

  // Deprecate the old data in block[block_id] and reallocate a new block
  virtual char* Reallocate(const uint64_t size, const int block_id);
};

class QueryAllocator : public MemAllocator {
 public:
  QueryAllocator() : MemAllocator(0) {}  // Same as MergeAllocator
  explicit QueryAllocator(const uint64_t default_block_size)
      : MemAllocator(default_block_size) {}

  QueryAllocator(const QueryAllocator&) = delete;
  QueryAllocator& operator=(const QueryAllocator&) = delete;

  virtual ~QueryAllocator() {}

  char* Allocate(const uint64_t size) { return MemAllocator::Allocate(size); }
};

class CacheAllocator : public MemAllocator {
 public:
  CacheAllocator() : MemAllocator() {}
  explicit CacheAllocator(const uint64_t default_block_size)
      : MemAllocator(default_block_size) {}

  CacheAllocator(const CacheAllocator&) = delete;
  CacheAllocator& operator=(const CacheAllocator&) = delete;

  virtual ~CacheAllocator() {}

  char* Allocate(const uint64_t size) { return MemAllocator::Allocate(size); }

  // Allocate the requested size of space and return block_id by reference
  char* Allocate(const uint64_t size, uint32_t& block_id);
};

#endif