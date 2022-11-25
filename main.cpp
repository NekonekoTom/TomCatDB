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
#include "db.h"
#include "db_table.h"
#include "mem_allocator.h"
#include "reader.h"
#include "sequence.h"
#include "skiplist.h"
#include "thread_pool.h"
#include "varint.h"

int main() {
  // TCDB db(Config());
  Config config;
  TCDB db(config);

  db.Insert(Sequence("kk352"), Sequence("uuha"));
  db.Insert(Sequence("ak322"), Sequence("auha"));

  auto ret = db.EntrySet();

  db.TestEntryPoint();

  SequentialReader sr(8);

  DBFile* file = new DBFile("./db/MANIFEST", DBFile::Mode::kReadOnly);

  std::string retstr;
  Status retstatu = sr.Read(file, retstr, 12);

  return 0;
}