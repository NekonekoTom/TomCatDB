/**
 * @file perf_test.cc
 * @author TomCat (NekonekoTom@outlook.com)
 * @brief Read and write performance test for TomCatDB, more details see doc
 * @version 0.1
 * @date 2023-08-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "db.h"

void ConcurrentWriteTest(TCDB& db, const int scale, const int thread_num) {
  std::cout << "<< Running concurrent write test >>\n";
  auto w_task = [&](int id) {
    for (int i = 0; i < scale; ++i) {
      db.Insert(
          Sequence("key_task_" + std::to_string(id) + "_" + std::to_string(i)),
          Sequence("key_task_" + std::to_string(id) + "_" + std::to_string(i)));
    }
  };

  std::vector<std::thread*> thread_ctrl;
  // std::clock() will fail under multi-thread environment
  // clock_t begin = std::clock(), end;
  // end = std::clock();
  auto begin = std::chrono::steady_clock::now();
  for (int i = 1; i <= thread_num; ++i) {
    std::thread* t = new std::thread(w_task, i);
    thread_ctrl.push_back(t);
  }

  for (auto t_ptr : thread_ctrl) {
    t_ptr->join();
    delete t_ptr;
  }
  auto end = std::chrono::steady_clock::now();
  auto write_time =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();  // in ms
  std::cout << "Write time: " + std::to_string(write_time) +
                   " ms (total_kv_num = " + std::to_string(scale * thread_num) +
                   ", thread_num = " + std::to_string(thread_num) + ")\n";
}

void ConcurrentReadTest(TCDB& db, const int scale, const int thread_num) {
  // Note: the bloom filter WILL NOT HIT!

  // Run concurrent write before read test
  ConcurrentWriteTest(db, scale, thread_num);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  std::cout << "<< Running concurrent read test >>\n";
  std::mutex r_m;
  auto r_task = [&](int tid) {
    std::string tmp;
    r_m.lock();
    db.Log(std::string("Start read task_") + std::to_string(tid));
    r_m.unlock();
    for (int ti = 0; ti < scale; ++ti) {
      tmp = db.Get(Sequence("key_task_" + std::to_string(tid) + "_" +
                            std::to_string(ti)));
      if (tmp.empty()) {
        db.Log(std::string("Tring to get key <key_task_") +
               std::to_string(tid) + "_" + std::to_string(ti) + "> failed.");
      }
    }
  };

  std::vector<std::thread*> thread_ctrl;
  auto begin = std::chrono::steady_clock::now();
  for (int i = 1; i <= thread_num; ++i) {
    std::thread* t = new std::thread(r_task, i);
    thread_ctrl.push_back(t);
  }

  for (auto t_ptr : thread_ctrl) {
    t_ptr->join();
    delete t_ptr;
  }
  auto end = std::chrono::steady_clock::now();

  auto read_time =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();  // in ms
  std::cout << "Read time: " + std::to_string(read_time) +
                   " ms (total_kv_num = " + std::to_string(scale * thread_num) +
                   ", thread_num = " + std::to_string(thread_num) + ")\n";
}

void SequentialWriteTest(TCDB& db, const int scale) {
  std::cout << "<< Running sequential write test >>\n";
  clock_t begin = std::clock(), end;
  for (int i = 0; i < scale; ++i) {
    db.Insert(Sequence("sequential_write_test_key_" + std::to_string(i)),
              Sequence("sequential_write_test_value_" + std::to_string(i)));
    }
  end = std::clock();
  auto write_time =
      static_cast<double>(end - begin) / CLOCKS_PER_SEC * 1000;  // in ms
  std::cout << "Write time: " + std::to_string(write_time) +
                   " ms (total_kv_num = " + std::to_string(scale) +
                   ", sequential)\n";
}

void SequentialReadTest(TCDB& db, const int scale) {
  // Note: the bloom filter WILL NOT HIT!

  // Run concurrent write before read test
  SequentialWriteTest(db, scale);

  std::cout << "<< Running sequential read test >>\n";
  clock_t begin = std::clock(), end;
  std::string tmp;
  for (int i = 0; i < scale; ++i) {
    tmp = db.Get(Sequence("sequential_write_test_key_" + std::to_string(i)));
    if (tmp.empty()) {
      db.Log(std::string("Tring to get key <") + "sequential_write_test_key_" +
             std::to_string(i) + "> failed.");
    }
  }
  end = std::clock();
  auto read_time =
      static_cast<double>(end - begin) / CLOCKS_PER_SEC * 1000;  // in ms
  std::cout << "Read time: " + std::to_string(read_time) +
                   " ms (total_kv_num = " + std::to_string(scale) +
                   ", sequential)\n";
}

int main(int argc, char* argv[]) {
  Config config;
  TCDB db(config);

  std::cout << "<< Running performance test >>\n";

  // ConcurrentReadTest(db, 25000, 4);

  SequentialReadTest(db, 1000000);

  return 0;
}