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

  // // Max (request_size > remaining_size_) times.
  // // The leftover space will be discarded and a fresh new block will be allocated
  // const int kMaxAllocFailTimes = 4;

  MemAllocator() : kMaxSectorSize(kDefaultBlockSize / 4) {
    start_addr_ = AllocateFullBlock(kDefaultBlockSize);
  }

  explicit MemAllocator(const uint64_t default_block_size)
      : kDefaultBlockSize(default_block_size),
        kMaxSectorSize(kDefaultBlockSize / 4) {
    start_addr_ = AllocateFullBlock(kDefaultBlockSize);
  }

  MemAllocator(const MemAllocator&) = delete;
  MemAllocator& operator=(const MemAllocator&) = delete;

  virtual ~MemAllocator();

  virtual char* Allocate(const uint64_t size);
  char* AllocateNewBlock(const uint64_t size);
  char* AllocateFullBlock(const uint64_t size);

  // Deprecate the old data in block[block_id] and reallocate a new block
  virtual char* Reallocate(const uint64_t size, const int block_id);

  void Ref(const std::vector<char*>::size_type block_id);

  const int Unref(const std::vector<char*>::size_type block_id);

  void RefLast(const int times) { ref.back() += times; }

  void RefBlock(const int times, const std::vector<char*>::size_type block_id) {
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
};

class MergeAllocator : public MemAllocator {
 public:
  // MergeAllocator() : MemAllocator(0) { block_size.push_back(0); }
  MergeAllocator() : MemAllocator(0) {}

  // Removed
  // explicit MergeAllocator(const uint64_t default_block_size)
  //     : MemAllocator(default_block_size) {}

  MergeAllocator(const MergeAllocator&) = delete;
  MergeAllocator& operator=(const MergeAllocator&) = delete;

  virtual ~MergeAllocator() {}

  char* Allocate(const uint64_t size) {
    // There is a one to one correspondence between a memory block and a DataBlock
    return MemAllocator::Allocate(size);
  }

  // Deprecate the old data in block[block_id] and reallocate a new block
  virtual char* Reallocate(const uint64_t size, const int block_id);

 private:
  // // Record the size of each block. This property is designed to calculate which
  // // block a char* ptr (start address of a Sequence) belongs to.
  // std::vector<uint64_t> block_size;
};

#endif