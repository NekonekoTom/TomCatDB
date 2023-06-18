/**
 * @file cache_test.cc
 * @author NekonekoTom (NekonekoTom@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <ctime>
#include "cache.h"
#include "csv.h"

int main(int argc, char* argv[]) {
  TCCache cache(2);

  int records = 20000;
  auto csv_data = CSVParser::ReadCSV(
      "/home/tom_cat/workdir/private/CS/C++/Primer/TomCatDB/test_data/test.csv",
      records);

  auto r1 = csv_data[1];
  auto r2 = csv_data[2];
  auto r3 = csv_data[3];

  std::string tmp;
  bool status = false;

  status = cache.Insert(r1[0], r1[1] + "-0");
  status = cache.Insert(r1[0], r1[1] + "-1");
  status = cache.Insert(r2[0], r2[1] + "-0");
  status = cache.Insert(r3[0], r3[1] + "-0");
  status = cache.Get(r1[0], tmp);
  status = cache.Delete(r3[0]);
  status = cache.Get(r2[0], tmp);
  status = cache.Get(r3[0], tmp);
  status = cache.Insert(r3[0], r3[1] + "-1");
  status = cache.Insert(r1[0], r1[1] + "-2");
  status = cache.Get(r1[0], tmp);
  status = cache.Get(r2[0], tmp);
  status = cache.Get(r3[0], tmp);

  return 0;
}