#include "mem_allocator.h"

MemAllocator::~MemAllocator() {
  for (auto ptr : block_ptr_) {
    delete ptr;
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
  return ret;
}