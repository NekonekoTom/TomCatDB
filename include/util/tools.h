#ifndef TOOLS_H_
#define TOOLS_H_

#include "base.h"

namespace neko_base
{

// Split std::string with char(seperator), return substrings in std::vector
std::vector<std::string> Split(const std::string& str, const char seperator);

// Join path as parent/child
std::string PathJoin(const std::string& parent, const std::string& child);

} // namespace neko_base


#endif