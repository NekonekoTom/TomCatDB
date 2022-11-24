#include "writer.h"

// This class manages all the file IOs for the TCDB
// A set of Writers is managed by the manager for reuse of writers
class IOManager {
 public:
  IOManager() = delete;

  IOManager(const IOManager&) = delete;
  IOManager& operator=(const IOManager&) = delete;

  IOManager(const std::string& files_dir);

  ~IOManager();

  Status WriteLevel0File();

 private:
  const int kDefaultLevel0FileNum = 4;

  const std::string kDatabaseDir;
  
  const std::string kManifestFilename = "MANIFEST";

  const std::string kLogFilename = "LOG";

  const std::string kSSTFilePostfix = ".sst";

  void BuildFile(DBFile** file_ptr, const std::string& filename);

  // Manifest file format as:
  // L1: relative path of the manifest file
  // L2: relative path of the log file
  DBFile* manifest_ = nullptr;

  DBFile* log_file_ = nullptr;

  std::vector<std::vector<DBFile*>> table_files_;

  // The writer for manifest file is at index 0;
  // The writer for log file is at index 1.
  std::vector<BaseWriter*> writers_;

  int file_levels_ = 0;
};