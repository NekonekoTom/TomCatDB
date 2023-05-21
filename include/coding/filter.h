#ifndef FILTER_H_
#define FILTER_H_

// The following equation gives the relations between m (bloom filter bits) and
// n (record numbers). p is the false-negative possibility of a query.
//   m = -n * ln(p)/(ln(2)*ln(2))
// The following equation gives the relations between k (hash function numbers)
// and m with n.
//   k = (m/n) * ln(2)
// The k is determined by m/n, and consequently, determined by p, the probability
// of a failed query:
//   k = -ln(p)/ln(2)
// In LevelDB (db_test.cc), m is calculated by m=n*bits_per_key, where a hyper-
// parameter bits_per_key is set to 10. It means one query of a nonexistent key
// using the bloom filter might have an approximately 0.82% chance of failing.

#include "base.h"
#include "hash.h"
#include "sequence.h"
#include "status.h"

class Filter {
 public:
  const double kFPRate = 0.0082;  // 0.82%, same as the LevelDB

  Filter() : fp_rate_(kFPRate) {}
  explicit Filter(const double fp_rate) : fp_rate_(fp_rate) {}

  virtual ~Filter() = default;

  virtual Status CreateFilter(const std::vector<Sequence>& entry_set,
                              std::string& filter_content) const = 0;

  virtual bool ContainsKey(const Sequence& key,
                             std::string& filter_content) const = 0;

  inline double fp_rate() const { return fp_rate_; }

 protected:
  const double kLn2 = 0.693;

 private:
  double fp_rate_;
};

class TCBloomFilter : public Filter {
 public:
  TCBloomFilter();
  explicit TCBloomFilter(const double fp_rate);

  virtual ~TCBloomFilter() = default;

  virtual Status CreateFilter(const std::vector<Sequence>& entry_set,
                              std::string& filter_content) const override;

  virtual bool ContainsKey(const Sequence& key,
                             std::string& filter_content) const override;

 private:
  void InitMembers();

  static std::vector<uint64_t> seeds_;

  int hash_k_;
  
  std::shared_ptr<Hasher> hasher_;
};

#endif