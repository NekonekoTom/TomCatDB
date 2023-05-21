#include "filter.h"
#include "internal_entry.h"

std::vector<uint64_t> TCBloomFilter::seeds_ = std::vector<uint64_t>{
    0x6fef439dc013aaa6, 0x089f55eb6baaab91, 0xfe28c5826fef439d,
    0x88606e7e6075f2c3, 0x38ed602f9a4f44a7, 0x45fc1384341b1bea,
    0x3de4e39c71165672, 0xae5f61af9a07fd46, 0x512f9502b7bc2663,
    0xae7ee2cb56659fc5};

TCBloomFilter::TCBloomFilter() : Filter() {
  InitMembers();
}

TCBloomFilter::TCBloomFilter(const double fp_rate) : Filter(fp_rate) {
  InitMembers();
}

Status TCBloomFilter::CreateFilter(const std::vector<Sequence>& entry_set,
                                   std::string& filter_content) const {
  Status ret;

  int bits =
      -static_cast<int>(entry_set.size()) / (kLn2 * kLn2) * std::log(fp_rate());

  int bytes = static_cast<int>(ceil(bits / 8.0));
  bits = bytes * 8;
  filter_content.resize(bytes, 0);

  uint64_t hash_value;
  decltype(filter_content.size()) pos = 0;
  for (auto& e : entry_set) {
    auto key = InternalEntry::EntryKey(e.data());

    for (int i = 0; i < hash_k_; ++i) {
      hash_value = hasher_->Hash(key.data(), key.size(), seeds_[i]);

      pos = (hash_value / 8) % filter_content.size();
      filter_content[pos] |=
          static_cast<unsigned char>(0x80) >> (hash_value % 8);
    }
  }

  return ret;
}

bool TCBloomFilter::ContainsKey(const Sequence& key,
                                std::string& filter_content) const {
  uint64_t hash_value;
  decltype(filter_content.size()) pos = 0;
  for (int i = 0; i < hash_k_; ++i) {
    hash_value = hasher_->Hash(key.data(), key.size(), seeds_[i]);

    pos = (hash_value / 8) % filter_content.size();
    if (!(filter_content[pos] &
          (static_cast<unsigned char>(0x80) >> (hash_value % 8))))
      return false;
  }

  return true;
}

void TCBloomFilter::InitMembers() {
  hash_k_ = -std::log(fp_rate()) / 0.693;  // ln(2) = 0.6931...

  while (seeds_.size() < hash_k_) {
    // TODO: lib function rand()'s return value ranges only 0x0 to 0x7ffffff,
    //       the highest bit is always 0.
#if __WORDSIZE == 64
    uint64_t one = static_cast<uint64_t>(std::rand())
                   << 32 + static_cast<uint64_t>(std::rand());
#else
    uint64_t one = static_cast<uint64_t>(std::rand())
                   << 48 + static_cast<uint64_t>(std::rand())
                   << 32 + static_cast<uint64_t>(std::rand())
                   << 16 + static_cast<uint64_t>(std::rand());
#endif
    seeds_.push_back(one);
  }

  hasher_ = std::make_shared<Murmur2>();
}