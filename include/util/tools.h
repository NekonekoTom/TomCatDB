#ifndef TOOLS_H_
#define TOOLS_H_

#include "base.h"

namespace neko_base {

// Split std::string with char(seperator), return substrings in std::vector
std::vector<std::string> Split(const std::string& str, const char seperator);

// Join path as parent/child
inline std::string PathJoin(const std::string& parent,
                            const std::string& child) {
  // TODO: Check legality?
  if (parent.back() == '/')
    return parent.substr(0, parent.size() - 1) + "/" + child;
  else
    return parent + "/" + child;
}

// Remove some elements from the std::vector src by given subscripts vector r_p.
// Param:
//   src: the vector to be changed;
//   r_p: r_p (remove points) gives the subscript vector of the elements to be
//        removed. The r_p MUST BE ASCENDING!
template <typename T>
void Remove(std::vector<T>& src,
            const std::vector<typename std::vector<T>::size_type>& r_p) {
  if (r_p.empty())
    return;
  if (r_p.size() == 1) {
    src.erase(src.begin() + r_p.front());
    return;
  }

  typename std::vector<T>::size_type removed = 0;
  auto first = src.begin();
  for (int i = 0; i < r_p.size() - 1; ++i) {
    std::copy(first + r_p[i] + 1, first + r_p[i + 1], first + r_p[i] - removed);
    ++removed;
  }
  // The last
  std::copy(first + r_p.back() + 1, src.end(), first + r_p.back() - removed);
  ++removed;

  // Erase invalid elements and resize
  src.erase(src.end() - removed, src.end());

  return;
}

}  // namespace neko_base

#endif