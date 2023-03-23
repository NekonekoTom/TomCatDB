#include "tools.h"

namespace neko_base {

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

bool RearrangeFilesInManifest(std::vector<std::string>& data_files,
                              const std::pair<int, int> boundary,
                              const std::vector<std::string>& new_files,
                              const int level) {
  std::vector<std::string> buffer;
  buffer.reserve(data_files.size() - (boundary.second - boundary.first) +
                 new_files.size());

  auto old_it = data_files.begin();

  if (boundary.first == boundary.second) {
    if (boundary.second == 0) {
      // Push front
      buffer.insert(buffer.end(), new_files.begin(), new_files.end());
      buffer.insert(buffer.end(), data_files.begin(), data_files.end());
    } else if (boundary.second == data_files.size()) {
      // Push back
      buffer.insert(buffer.end(), data_files.begin(), data_files.end());
      buffer.insert(buffer.end(), new_files.begin(), new_files.end());
    } else {
      // Wrong parameter
      return false;
    }
    data_files = std::move(buffer);
    return true;
  }

  // Insert SST files that smaller than the new files
  buffer.insert(buffer.end(), old_it, old_it + boundary.first);

  // Insert new SST files
  buffer.insert(buffer.end(), new_files.begin(), new_files.end());

  // Insert SST files that larger than the new files
  buffer.insert(buffer.end(), old_it + boundary.second, data_files.end());
  // Bug: old_it + boundary.second or old_it + boundary.second - 1?

  data_files = std::move(buffer);

  return true;
}

}  // namespace neko_base