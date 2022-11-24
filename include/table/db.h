#include "raii_lock.h"
#include "db_table.h"
#include "writer.h"
#include "io_manager.h"
#include "config.h"

class TCDB {
 public:
  TCDB(const Config& config);

  TCDB(const TCDB&) = delete;
  TCDB& operator=(const TCDB&) = delete;

  ~TCDB();

  const Sequence* Get(const Sequence& key);

  Status Insert(const Sequence& key, const Sequence& value);

  Status Delete(const Sequence& key);

  bool ContainsKey(const Sequence& key);

  // For test
  const std::vector<const char*> EntrySet() { return volatile_table_->EntrySet(); }

  // For test: compactions, write files...
  void TestEntryPoint() {
    WriteLevel0();
  }

 private:
  Status TransferTable(const TCTable** immutable);

  // This function is triggered when the volatile_table_ reaches max size
  Status WriteLevel0();

  std::mutex mutex_; // Basic mutex

  RAIILock global_lock_;

  TCTable* volatile_table_;

  IOManager io_manager_;
};