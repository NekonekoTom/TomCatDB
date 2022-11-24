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

  ~MemAllocator();

  char* Allocate(const uint64_t size);
  char* AllocateNewBlock(const uint64_t size);
  char* AllocateFullBlock(const uint64_t size);

 private:
  uint64_t remaining_size_ = kDefaultBlockSize;
  char* start_addr_ = nullptr;

  std::vector<char*> block_ptr_;
};

#endif