#include "mem_allocator.h"

MemAllocator::~MemAllocator() {
  for (auto& ptr : block_ptr_) {
    if (ptr != nullptr)
      delete[] ptr;
  }
}

char* MemAllocator::Allocate(const uint64_t size) {
  char* ret = nullptr;
  if (size > kMaxSectorSize) {
    // Too big for kDefaultBlockSize
    return AllocateFullBlock(size);
  }
  if (size > remaining_size_) {
    // Waste the leftover unallocated space
    return AllocateNewBlock(size);
  } else {
    // Allocate a tiny block
    ret = start_addr_;
    start_addr_ += size;
    remaining_size_ -= size;
    ++ref.back();
  }
  return ret;
}

char* MemAllocator::AllocateNewBlock(const uint64_t size) {
  char* ret = AllocateFullBlock(kDefaultBlockSize);
  start_addr_ = ret + size;
  remaining_size_ = kDefaultBlockSize - size;
  return ret;
}

char* MemAllocator::AllocateFullBlock(const uint64_t size) {
  char* ret = new char[size];
  block_ptr_.push_back(ret);
  ref.push_back(1);
  return ret;
}

char* MemAllocator::Reallocate(const uint64_t size, const int block_id) {
  // Not available
  return nullptr;
}

void MemAllocator::Ref(const std::vector<char*>::size_type block_id) {
  if (block_id < ref.size())
    ++ref[block_id];
}

const int MemAllocator::Unref(const std::vector<char*>::size_type block_id) {
  if (block_id < ref.size())
    return --ref[block_id];
  else
    return -1;
}

void MemAllocator::ReleaseIdleSpace() {
  for (int i = 0; i < ref.size(); ++i) {
    if (ref[i] == 0) {
      delete[] block_ptr_[i];
      block_ptr_[i] = nullptr;
    }
  }
}

void MemAllocator::ReleaseIdleSpace(const int block_id) {
  assert(block_id < ref.size() && ref[block_id] == 0);
  delete[] block_ptr_[block_id];
  block_ptr_[block_id] = nullptr;
}

// Deprecate the old data in block[block_id] and reallocate a new block
char* MergeAllocator::Reallocate(const uint64_t size, const int block_id) {
  ReleaseIdleSpace(block_id);

  // Always allocate a full block in MergeAllocator. In this design, parameter
  // <size> be large enough to avoid possible performance loss.
  // TODO: Optimization?
  block_ptr_[block_id] = new char[size];
  ++ref[block_id];

  return block_ptr_[block_id];
}