#include "tools.h"

namespace neko_base
{

std::vector<std::string> Split(const std::string& str, const char seperator) {
  std::vector<std::string> ret;
  std::size_t start = 0, end = 0;
  while ((end = str.find(seperator, start)) != std::string::npos) {
    ret.push_back(str.substr(start, end - start));
    start = end + 1;
  }
  std::string last = str.substr(start, end - start);
  if (last.size() > 0)
    ret.push_back(last);
  return ret;
}

std::string PathJoin(const std::string& parent, const std::string& child) {
  // TODO: Check legality?
  if (parent.back() == '/')
    return parent.substr(0, parent.size() - 1) + "/" + child;
  else
    return parent + "/" + child;
}

} // namespace neko_base