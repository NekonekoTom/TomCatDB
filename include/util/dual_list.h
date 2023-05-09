#ifndef DUAL_LIST_H_
#define DUAL_LIST_H_

#include "base.h"

namespace neko_base {

template <typename T>
class DualList {
 public:
  typedef uint64_t size_type;
  struct DualListNode {
    DualListNode() : prev(nullptr), next(nullptr) {}
    DualListNode(const T& t) : DualListNode(t, nullptr, nullptr) {}
    DualListNode(const T& t, DualListNode* prv, DualListNode* nxt)
        : data(t), prev(prv), next(nxt) {}

    T data;  // Uninitialized
    DualListNode* prev;
    DualListNode* next;
  };

  DualList() : head_(nullptr), tail_(nullptr), size_(0) {}
  virtual ~DualList() {}

  virtual void MoveFront(DualListNode* node) {
    assert(node != nullptr);

    // Cut connections
    node->prev->next = node->next;
    node->next->prev = node->prev;

    // Move front
    node->next = head_->next;
    node->prev = head_;
    head_->next->prev = node;
    head_->next = node;
  }

  virtual DualListNode* PushFront(const T& t) {
    DualListNode* node = new DualListNode(t);
    if (size_ == 0) {
      InitHeadTailNodes();
    }
    node->next = head_->next;
    node->next->prev = node;

    head_->next = node;
    node->prev = head_;
    
    ++size_;
    return node;
  }

  virtual void PopBack() {
    DualListNode* prev_node = tail_->prev;
    tail_->prev = prev_node->prev;
    tail_->prev->next = tail_;
    delete prev_node;

    if (size_ > 0)
      --size_;
  }

  // TODO
  virtual void Reserve() {}

  virtual const T& Back() const { return tail_->prev->data; }

  inline size_t size() const { return size_; }

 private:
  void InitHeadTailNodes() {
    head_ = new DualListNode();
    tail_ = new DualListNode();
    head_->next = tail_;
    tail_->prev = head_;
  }

  DualListNode* head_;  // Dummy head node
  DualListNode* tail_;  // Dummy tail node
  size_type size_;
};

}  // namespace neko_base

#endif