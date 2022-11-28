#include "db_table.h"
#include "format.h"
#include "reader.h"
#include "tools.h"
#include "writer.h"

// This class manages all the file IOs for the TCDB
// A set of Writers is managed by the manager for reuse of writers
class IOManager {
 public:
  IOManager() = delete;

  IOManager(const IOManager&) = delete;
  IOManager& operator=(const IOManager&) = delete;

  // TODO: Construct by Config object?
  explicit IOManager(const std::string& files_dir, RAIILock& io_lock);

  ~IOManager();

  Status WriteLevel0File(const TCTable* immutable);

  // TODO: Argument?
  // Status WriteSSTFile(const std::vector<const char*> entry_set);

 private:
  const int kDefaultLevel0FileNum = 4;

  const int kDefaultReaderBufferSize = 4096;

  // kDefaultWriterBufferSize is for BatchWriter that writes data blocks into
  // the SST file. Default writer buffer size should be equal to default block
  // size:
  // IOManager::kDefaultWriterBufferSize == DataFileFormat::kDefaultDataBlkSize
  const int kDefaultWriterBufferSize = 4096;

  // Construct kDefaultReaderNum SequentialReader object in readers_ vector
  const int kDefaultReaderNum = 4;

  const std::string kDatabaseDir;

  const std::string kManifestFilename = "MANIFEST";

  const std::string kLogFilename = "LOG";

  const std::string kSSTFilePostfix = ".tdb";

  void BuildMetadataFile(const std::string& file_name);

  ManifestFormat::ManifestData ReadManifest(Status& read_status);

  // Write entry_set to specified SST file. Call vector<Sequence> version.
  Status WriteSSTFile(const std::string& file_name,
                      const std::vector<const char*>& entry_set);

  // Write entry_set to specified SST file.
  Status WriteSSTFile(const std::string& file_name,
                      const std::vector<Sequence>& entry_set);

  // Write data blocks to SST file. Called by WriteSSTFile()
  Status WriteSSTData(std::shared_ptr<BatchWriter>& bw_ptr,
                      const std::vector<Sequence>& entry_set,
                      std::vector<uint32_t>& data_blk_offset,
                      uint32_t& data_block_size);

  // Write data block offset to SST file. Called by WriteSSTFile()
  Status WriteSSTIndex(std::shared_ptr<BatchWriter>& bw_ptr,
                       const std::vector<uint32_t>& data_blk_offset);

  // Write flexible to SST file. Called by WriteSSTFile()
  // TODO: crc-32?
  Status WriteSSTFlexible(std::shared_ptr<BatchWriter>& bw_ptr);

  // TODO: Call this function after all entries are successfully written
  Status WriteSSTFileFooter(std::shared_ptr<BatchWriter>& bw_ptr,
                            const uint32_t data_block_size,
                            const uint32_t index_block_size,
                            const uint32_t flexible_block_size);

  std::vector<std::vector<DBFile*>> table_files_;

  // Reader workers for sequential reading
  std::vector<SequentialReader*> readers_;

  // The writer for manifest file is at index 0;
  // The writer for log file is at index 1.
  std::vector<BaseWriter*> writers_;

  int file_levels_ = 0;

  // Use file id for .tdb file names.
  // For example, file_id_(0x35AC186F) refers to data file 0000000035AC186F.tdb
  uint64_t file_id_ = 0;

  RAIILock& io_lock_;
};