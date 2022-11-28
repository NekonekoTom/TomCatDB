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
#include "tools.h"

int main() {
  // TCDB db(Config());
  Config config;
  TCDB db(config);

  db.Insert(Sequence("kk352"), Sequence("uuha")); // InternalEntry size: 20
  db.Insert(Sequence("ak322"), Sequence("auha"));
  db.Delete(Sequence("ak322"));                   // InternalEntry size: 15

  auto entry_set = db.EntrySet();

  SequentialReader sr(8);

  std::string retstr;
  Status retstatu = sr.Read(new DBFile("./db/MANIFEST", DBFile::Mode::kReadOnly), retstr, 12);

  retstatu = sr.Read(new DBFile("./db/MANIFEST", DBFile::Mode::kReadOnly), retstr, 97);

  retstatu = sr.ReadEntire(new DBFile("./db/MANIFEST", DBFile::Mode::kReadOnly), retstr);

  db.TestEntryPoint();

  return 0;
}