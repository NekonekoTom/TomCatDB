/**
 * @file main.cpp
 * @author TomCat (noemail@noemail.com)
 * @brief Main test
 * @version 0.1
 * @date 2022-09-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <ctime>
#include "comparator.h"
#include "csv.h"
#include "db.h"
#include "db_table.h"
#include "mem_allocator.h"
#include "reader.h"
#include "sequence.h"
#include "skiplist.h"
#include "thread_pool.h"
#include "tools.h"
#include "varint.h"

int main() {
  Config config;
  TCDB db(config);

  // TCThreadPool thread_pool(4, 10);
  // thread_pool.Start();

  // auto job = [](int x, int y) {
  //   std::cout << "Computing...\n";
  //   std::this_thread::sleep_for(std::chrono::milliseconds(200));
  //   std::cout << "Done...\n";
  //   return x + y;
  // };

  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // thread_pool.SubmitTask(job, 1, 2);
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  // auto i_want_x_plus_y = thread_pool.SubmitTask(job, 1, 2);

  // auto res_x_y = i_want_x_plus_y.get();
  // std::cout << "Got result: " << res_x_y << std::endl;

  // thread_pool.Shutdown();

  int records = 20000;
  auto csv_data = CSVParser::ReadCSV(
      "/home/tom_cat/workdir/private/CS/C++/Primer/TomCatDB/test_data/test.csv",
      records);

  Status why_status;
  std::string why;
  clock_t begin = std::clock(), end;
  // for (int circle = 0; circle < 20; ++circle) {
  for (int circle = 0; circle < 5; ++circle) {
    for (int i = 0; i < records; ++i) {
      db.Insert(csv_data[i][0] + "Circle-" + std::to_string(circle), csv_data[i][10]); // id and description
      // why_status = db.Insert(csv_data[i][0] + "Circle-" + std::to_string(circle), csv_data[i][20]); // id and name
      if (!why_status.StatusNoError())
        std::cout << i << why_status.ErrMsg() << std::endl;
      // db.Insert(csv_data[i][0], csv_data[i][10]); // id and description
      // db.Insert(csv_data[i][0], csv_data[i][20]);  // id and name
    }
  }
  end = std::clock();

  auto write_time = static_cast<double>(end - begin) / CLOCKS_PER_SEC * 1000; // in ms
  
  std::cout << "Write time: " + std::to_string(write_time) + " ms\n";

  auto get_test = db.Get(Sequence("idCircle-1"));

  std::cout << "Get: " << get_test << std::endl;

  // Error get test
  begin = std::clock();
  int error = 0;
  for (int circle = 0; circle < 5; ++circle) {
    for (int i = 0; i < records; ++i) {
      auto test_key = csv_data[i][0] + "Circle-" + std::to_string(circle);
      why = db.Get(test_key); // id and description
      // if (why.empty()) {
      //   db.Log("Try to get key [" + test_key + "] error. Result empty.");
      //   ++error;
      // }
    }
  }
  end = std::clock();
  auto read_time = static_cast<double>(end - begin) / CLOCKS_PER_SEC * 1000; // in ms
  std::cout << "Read time: " + std::to_string(read_time) + " ms\n";

  int correct = 0;
  for (int i = 0; i < 20; ++i) {
    // if (!db.Get(std::string("idCircle-" + std::to_string(i))).empty()) {
    auto fk = db.Get(std::string("13911206Circle-" + std::to_string(i)));
    if (!fk.empty()) {
      ++correct;
    }
  }

  // auto val = db.Get(std::string("14402792"));
  auto val1 = db.Get(std::string("idCircle-3"));
  auto val2 = db.Get(std::string("3895911Circle-3"));
  auto val3 = db.Get(std::string("13911206Circle-3"));
  auto val4 = db.Get(std::string("13911206Circle-7"));
  // auto val1 = db.Get(std::string("id"));
  // auto val2 = db.Get(std::string("3895911"));
  // auto val3 = db.Get(std::string("13911206"));

  // db.TestEntryPoint();
  return 0;
}