#ifndef SKIPLIST_H_
#define SKIPLIST_H_

#include "base.h"
#include "comparator.h"
#include "random.h"

// template <typename K>
// class SkipList;

template <typename K>
struct SkipListNode {
  SkipListNode(const K& key, const int size)
      : key_(key), next_(size > 0 ? new SkipListNode<K>*[size] : nullptr) {}

  ~SkipListNode() {
    if (next_ != nullptr)
      delete[] next_;
  }

  const K key_;

  // The value member is unnecessary since we designed to store an entire Record
  // as an entry in the SkipList. The actual value can be decoded from a Record
  // V value_;

  // Cannot declare like this since length of the second demension is unknown
  // SkipListNode<K>* next_[];
  SkipListNode<K>** next_;
};

template <typename K, class Cmp = Comparator<int>>
class SkipList {
 public:
  SkipList(const std::shared_ptr<Cmp>&, const K& last_key);

  SkipList(const SkipList&) = delete;
  SkipList& operator=(const SkipList&) = delete;

  ~SkipList();

  // Get node by key
  const SkipListNode<K>* Get(const K& key) const;

  // Insert a key-value pair
  // Return:
  //   -1 : Not inserted caused by error
  //    0 : Inserted new node successfully
  //    1 : Value updated
  const int Insert(const K& key);

  const std::vector<K> EntrySet() const;

  // For test
  const int NodeCount() {
    SkipListNode<K>* p = head_;
    int count = 0;
    while (p != tail_) {
      ++count;
      p = p->next_[0];
    }
    return count + 1;
  }

  // For test
  bool IsNodeAscend() {
    SkipListNode<K>* p = head_;
    while (p->next_[0] != tail_) {
      if (comparator_->Greater(p->key_, p->next_[0]->key_))
        return false;
      p = p->next_[0];
    }
    return true;
  }

  // Access functions
  const int size() const { return size_; }
  const int levels() const { return levels_; }

 private:
  const int kMaxLevels = 12;

  // Search key in the skiplist and store last visited node at level 'i' in last_node_[i].
  // Return a "before node" pointer, pointing to the node including specific key or just less then key.
  SkipListNode<K>* Search(const K& key) const;

  // Allocate a random level
  const int RandomLevel() const;

  // No last_key_ member in our design
  // const K& last_key() const { return last_key_; }

  int size_;
  int levels_;

  // No last_key_ member in our design
  // K last_key_;

  SkipListNode<K>* head_;
  SkipListNode<K>* tail_;

  // last_node[i] is for the last visited node at level 'i' after executing Search()
  // Cannot declare like this since length of the second demension is unknown
  // SkipListNode<K>* last_node_[];
  SkipListNode<K>** last_node_;

  const std::shared_ptr<Cmp>& comparator_;
};

template <typename K, class Cmp>
SkipList<K, Cmp>::SkipList(const std::shared_ptr<Cmp>& comparator, const K& last_key)
    : size_(0), levels_(0), comparator_(comparator) {

  // // K must have default constructor
  // K key;  // Dummy key

  head_ = new SkipListNode<K>(last_key, kMaxLevels + 1);
  tail_ = new SkipListNode<K>(last_key, 0);
  last_node_ = new SkipListNode<K>*[kMaxLevels + 1];

  // Initialize head nodes
  for (int i = 0; i < kMaxLevels; ++i) {
    head_->next_[i] = tail_;
  }
}

template <typename K, class Cmp>
SkipList<K, Cmp>::~SkipList() {
  if (head_ != nullptr) {
    delete head_;
  }
  if (tail_ != nullptr) {
    delete tail_;
  }
  if (last_node_ != nullptr) {
    delete[] last_node_;
  }
}

template <typename K, class Cmp>
const SkipListNode<K>* SkipList<K, Cmp>::Get(const K& key) const {
  // The closest node to key node on the left
  SkipListNode<K>* before_node = head_;

  for (int i = levels_; i >= 0; --i) {
    while (before_node->next_[i] != tail_ &&
           !(comparator_->GreaterOrEquals(before_node->next_[i]->key_, key))) {
      before_node = before_node->next_[i];
    }
  }

  // The correct implementation of the SkipList should be as follows:
  // **********************************************************
  // if (comparator_->Equal(before_node->next_[0]->key_, key)) {
  //   return before_node->next_[0];
  // }
  // **********************************************************
  // However, in our implementation, the before_node will point to the
  // last key that "Less" than the given key. Since the given key does
  // not actually exist in the SkipList and the id is set to MAX
  // uint64_t, to check if the key is in the SkipList, we should compare
  // the key with the before_node's key instead of before_node->next_[0]'s
  // in normal implementation.
  if (comparator_->Equal(before_node->key_, key)) {
    return before_node;
  }

  return nullptr;
}

template <typename K, class Cmp>
SkipListNode<K>* SkipList<K, Cmp>::Search(const K& key) const {
  SkipListNode<K>* before_node = head_;
  for (int i = levels_; i >= 0; --i) {
    while (before_node->next_[i] != tail_ &&
           !(comparator_->GreaterOrEquals(before_node->next_[i]->key_, key))) {
      before_node = before_node->next_[i];
    }
    last_node_[i] = before_node;
  }
  return before_node->next_[0];
}

template <typename K, class Cmp>
const int SkipList<K, Cmp>::Insert(const K& key) {
  SkipListNode<K>* node = Search(key);
  // Commented when testing comparator_->Equal for Get
  // if (comparator_->Equal(node->key_, key)) {
  //   // Key exists, update value

  //   // No value member in our design
  //   // node->value_ = value;  // operator= triggered
  //   return 1;
  // }

  int level = RandomLevel();
  if (level > levels_) {
    level = ++levels_;
    last_node_[level] = head_;
  }

  // Insert
  SkipListNode<K>* new_node = new SkipListNode<K>(key, level + 1);
  for (int i = 0; i <= level; ++i) {
    new_node->next_[i] = last_node_[i]->next_[i];
    last_node_[i]->next_[i] = new_node;
  }

  ++size_;
  return 0;
}

template <typename K, class Cmp>
const std::vector<K> SkipList<K, Cmp>::EntrySet() const {
  std::vector<K> ret;

  SkipListNode<K>* p = head_->next_[0];
  while (p != tail_) {
    ret.push_back(p->key_);
    p = p->next_[0];
  }

  return ret;
}

template <typename K, class Cmp>
const int SkipList<K, Cmp>::RandomLevel() const {
  int lev = 0;

  assert(kMaxLevels > 1);
  while (neko::SelectedInProb1DivdN(4))
    ++lev;
  return (lev <= kMaxLevels) ? lev : kMaxLevels;
}

#endif