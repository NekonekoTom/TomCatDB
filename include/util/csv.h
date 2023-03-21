#ifndef CSV_H_
#define CSV_H_

#include <fstream>
#include <unordered_map>
#include "tools.h"

class CSVParser {
 public:
  ~CSVParser() = default;

  // Read "lines" lines of the specified .csv file
  static std::vector<std::vector<std::string>> ReadCSV(
      const std::string& filename, const int lines);

  // Read entire .csv file
  static std::vector<std::vector<std::string>> ReadCSV(
      const std::string& filename);

 private:
  // Seperator for columns
  static const char kSeperator = ',';

  // A string is represented by: kStringBoundaryFlag + content + kStringBoundaryFlag
  static const char kStringBoundaryFlag = '"';

  CSVParser() = delete;
  CSVParser(const CSVParser&) = delete;
  CSVParser& operator=(const CSVParser&) = delete;

  // Split one line to columns
  static std::vector<std::string> SplitCol(const std::string&);
};

#endif