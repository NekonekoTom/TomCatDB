#include "cache.h"
#include "config.h"
#include "db_table.h"
#include "io.h"
#include "raii_lock.h"
#include "thread_pool.h"
#include "writer.h"

class TCDB {
 public:
  // Default SST file size, also as max TCTable size.
  // When the capacity reaches the limit, the TCTable will be transferred
  // to an immutable table and written to level 0 SST file for persistence.
  const int kDefaultSSTFileSize =
      DataFileFormat::kApproximateSSTFileSize;  // 2MB

  // const int kDefaultLevel0FileNum = 4;

  // Max level for SST files. When kMaxLevel == 12, the max level number is 11.
  const int kMaxLevel = 12;  // TODO: Construct by Config

  // Default max files number at each level. This property should be constructed
  // in the construction function. kDefaultLevelSize can be calculated by rules
  // as follows: level 0 and level 1 size can be arbitrary, but level 0 should
  // be smaller than level 1; from the third level, we have:
  //                      level(i + 1) = level(i) * E
  // where E is an exponential specified by the user. In the LevelDB, E = 10
  // and level(0) = 4, level(1) = 10.
  std::vector<long> kDefaultLevelSize{
      4,           10,          100,      1000,      10000,
      100000,      1000000,     10000000, 100000000, 1000000000,
      10000000000, 100000000000};  // TODO:Construct by Config

  TCDB(const Config& config);
  TCDB(const TCDB&) = delete;
  TCDB& operator=(const TCDB&) = delete;

  ~TCDB();

  std::string Get(const Sequence& key);

  Status Insert(const Sequence& key, const Sequence& value);

  Status Delete(const Sequence& key);

  bool ContainsKey(const Sequence& key);

  Status Log(const std::string& msg) { return io_.Log(msg); }

  // For test
  const std::vector<const char*> EntrySet() {
    return volatile_table_->EntrySet();
  }

  // For test: compactions, write files...
  void TestEntryPoint() {
    // WriteLevel0();  // Done

    ManifestFormat::ManifestData manifest;
    io_.ReadManifest(manifest);

    Status ret = BackgroundCompact(manifest);

    // DataFileFormat::Footer footer;
    // io_.ReadSSTFooter(manifest.data_files[0][0], footer);
  }

 private:
  Status TransferTable(const TCTable** immutable);

  // This function is triggered when the volatile_table_ reaches max size
  Status WriteLevel0();

  // Start background compaction, and return whether the compaction process was
  // successfully started. The compaction process may take a lot of time, so
  // any operation would be recorded in the LOG file and executed after the
  // compaction is finished.
  Status BackgroundCompact(ManifestFormat::ManifestData& manifest);

  // Compact the SST file. Find all overlapped SST files at current level (if
  // current_level == 0) and the next level (current_level + 1), make a vector
  // that includes all overlapped files, and then call MultiwayMerge().
  Status CompactSST(ManifestFormat::ManifestData& manifest,
                    const int current_level, const int compact_file_num);

  // Compact the SST file. Find all overlapped SST files at current level (if
  // current_level == 0) and the next level (current_level + 1), make a vector
  // that includes all overlapped files, and then call MultiwayMerge().
  // Note: all files are at the same level and MUST be continuous!
  Status CompactSST(ManifestFormat::ManifestData& manifest,
                    const int current_level,
                    const std::vector<int>& compact_file_num);

  // Called by BackgroundCompact(). This function initializes the multiway
  // merge process by reading the index blocks of the compact files and pushing
  // the first DataBlock of each file into the priority queue. The real merging
  // process will be done in IterMerge().
  Status MultiwayMerge(const std::vector<std::string>& compact_file_abs_path,
                       std::vector<std::string>& new_files);

  // Iterately merge the files
  // Params:
  //   compact_files_abs_path: compaction files absolute path;
  //   priority_queue: see the implementation of TCDB::MultiwayMerge();
  //   merge_allocator: the mem pool that takes control of the real entry data;
  //   file_entry_in_queue: file_entry_in_queue[i] denotes the number of
  //                        entries from the i-th file in the queue;
  //   index_list: IndexBlock info;
  //   new_files: newly written SST files, basename only.
  Status IterMerge(
      const std::vector<std::string>& compact_files_abs_path,
      std::priority_queue<std::tuple<Sequence, int, int>,
                          std::vector<std::tuple<Sequence, int, int>>,
                          MergeComparator>& priority_queue,
      std::shared_ptr<MemAllocator>& merge_allocator,
      std::vector<int>& file_entry_in_queue,
      const std::vector<std::vector<uint32_t>>& index_list,
      std::vector<std::string>& new_files);

  // Search the query_key on the specified level
  Status SearchLevel(const Sequence& query_key, Sequence& ret_key,
                     const ManifestFormat::ManifestData& manifest,
                     const int level);

  // Get query_key and corresponding value from the SST file.
  // If the query_key exists, it should be unique and the searching process
  // will exit once a "Equal" key is found.
  Status GetFromSST(const Sequence& query_key, Sequence& ret_key,
                    const std::string& file_abs_path,
                    const DataFileFormat::Footer& footer);

  std::mutex mutex_;  // Basic mutex

  RAIILock global_lock_;

  std::shared_ptr<InternalEntryComparator> comparator_;

  TCTable* volatile_table_;

  std::shared_ptr<Filter> filter_;

  TCIO io_;

  TCThreadPool worker_;

  // Cache module for the keys and values. Both key and value should be of type
  // Sequence, the Cache is in charge of underlying storage for the Sequence.
  std::shared_ptr<Cache<Sequence, Sequence>> cache_;

  std::shared_ptr<MemAllocator> query_buffer_;
};