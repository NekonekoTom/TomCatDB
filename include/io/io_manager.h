#include "reader.h"
#include "writer.h"
#include "format.h"

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

  const int kDefaultReaderBufferSize = 4096;

  // Construct kDefaultReaderNum SequentialReader object in readers_ vector
  const int kDefaultReaderNum = 1;

  const std::string kDatabaseDir;
  
  const std::string kManifestFilename = "MANIFEST";

  const std::string kLogFilename = "LOG";

  const std::string kSSTFilePostfix = ".sst";

  void BuildMetadataFile(DBFile** file_ptr, const std::string& filename);

  // Manifest file format as:
  // L1: path of the manifest file
  // L2: path of the log file
  // L3: level 0     file names, seperated by ";"
  // Li: level (i-3) file names, seperated by ";"
  DBFile* manifest_ = nullptr;

  // Log file format as:
  // L1: path of the log file
  // Li: log entries started by uint_64t head(8B)
  DBFile* log_file_ = nullptr;

  std::vector<std::vector<DBFile*>> table_files_;

  // Reader workers for sequential reading
  std::vector<SequentialReader*> readers_;

  // The writer for manifest file is at index 0;
  // The writer for log file is at index 1.
  std::vector<BaseWriter*> writers_;

  int file_levels_ = 0;
};