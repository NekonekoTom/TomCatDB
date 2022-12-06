#include "csv.h"

std::vector<std::vector<std::string>> CSVParser::ReadCSV(
    const std::string& filename, const int lines) {
  std::string buffer;
  std::ifstream ifs(filename);
  std::vector<std::vector<std::string>> csv_data;

  for (int i = 0; i < lines; i++) {
    getline(ifs, buffer);

    csv_data.push_back(SplitCol(buffer));
  }
  ifs.close();

  return csv_data;
}

std::vector<std::vector<std::string>> CSVParser::ReadCSV(
    const std::string& filename) {
  std::string buffer;
  std::ifstream ifs(filename);
  std::vector<std::vector<std::string>> csv_data;

  getline(ifs, buffer);
  while (!buffer.empty()) {
    csv_data.push_back(SplitCol(buffer));

    getline(ifs, buffer);
  }
  ifs.close();

  return csv_data;
}

std::vector<std::string> CSVParser::SplitCol(const std::string& str) {
  std::vector<std::string> ret;

  if (str.empty()) {
    return ret;
  }

  int boundary_stack = 0;  // Similar to parenthesis matching
  std::string::size_type i = 0, start = 0;

  while (i < str.size()) {
    // Column start
    if (str[i] != kStringBoundaryFlag) {
      while (i < str.size() && str[i] != kSeperator)
        ++i;
      if (i == str.size()) {
        ret.push_back(str.substr(start, i - start - 1));
      } else {
        ret.push_back(str.substr(start, i - start));
      }
      ++i;
      start = i;
      // Column end
    } else {  // str[i] == kStringBoundaryFlag
      ++boundary_stack;
      ++i;
      ++start;

      while (boundary_stack != 0) {
        while (i < str.size() && str[i] != kStringBoundaryFlag)
          ++i;
        if (boundary_stack == 1) {
          if (i + 1 < str.size() &&
              (str[i + 1] == kSeperator || str[i + 1] == '\n')) {
            // Column end
            ret.push_back(str.substr(start, i - start));
            start = i + 2; // Skip the kSeperator and kStringBoundaryFlag
            ++i; // Skip kSeperator
            --boundary_stack;  // boundary_stack == 0, exit
          } else {
            ++boundary_stack;
          }
        } else {
          --boundary_stack;
        }
        ++i; // Skip kStringBoundaryFlag
      }
    }
  }

  return ret;
}