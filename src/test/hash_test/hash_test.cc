#include "filter.h"

// class Base {
//  public:
//   const int _a = 1312;
//   Base() : var(_a) {}
//   Base(int x) : var(x) {}

//   int var;
// };

// class Test : public Base {
//  public:
//   // Test() : a(var) {
//   Test() : Base(_a) {
//     printf("%d", var);
//   }
// };

int main() {
  std::string content{"Hash_test"};
  std::vector<Sequence> entry_set(10, content);
  std::string filter_content;

  Filter* filter = new TCBloomFilter();
  filter->CreateFilter(entry_set, filter_content);

  return 0;
}