#include "db_table.h"
#include "format.h"
#include "logger.h"
#include "reader.h"
#include "tools.h"
#include "writer.h"

// This class manages all the file IOs for the TCDB
// A set of Writers is managed by the manager for reuse of writers
class TCIO {
 public:
  const int kDefaultReaderBufferSize = 4096;

  // kDefaultWriterBufferSize is for SequentialWriter that writes data blocks into
  // the SST file. Default writer buffer size should be equal to default block
  // size:
  // TCIO::kDefaultWriterBufferSize == DataFileFormat::kDefaultDataBlkSize
  const int kDefaultWriterBufferSize = 4096;

  // Construct kDefaultReaderNum SequentialReader object in readers_ vector
  const int kDefaultReaderNum = 4;

  const std::string kDatabaseDir;

  const std::string kManifestFilename = "MANIFEST";

  const std::string kLogFilename = "LOG";

  const std::string kSSTFilePostfix = ".tdb";

  TCIO() = delete;

  TCIO(const TCIO&) = delete;
  TCIO& operator=(const TCIO&) = delete;

  // TODO: Construct by Config object?
  TCIO(const std::string& files_dir, RAIILock& io_lock);

  ~TCIO();

  Status WriteLevel0File(const TCTable* immutable,
                         ManifestFormat::ManifestData& manifest);

  // Different from the implementation of WriteLevel0File(), the caller determines
  // the level of the new SST file. This function only returns the SST file name
  // by reference. The caller function is responsible for collecting all changes
  // and merging changes in one MANIFEST.
  Status WriteNewSSTFile(const std::vector<Sequence>& entry_set,
                         std::string& file_basename);

  // Interface for MultiwayMerge().
  Status WriteMergeSSTFile(
      const std::vector<std::tuple<Sequence, int, int>>& item_set,
      std::string& file_basename,
      std::shared_ptr<MemAllocator>& merge_allocator);

  // Update the Manifest file after merging.
  // Params:
  //   old_manifest: it it what it is;
  //   compact_file_index: the element of the std::vector is of type
  //                       std::pair<int, int>, the first element denotes the
  //                       level of the compact file, and the second denotes
  //                       the number of it in its level;
  //   new_files: vector of std::string, basename only;
  //   current_level: the lowest level among the compact files.
  //   insert_index: new files will be inserted on the insert_index. Files
  //                 smaller than the new files are at [0, insert_index).
  Status UpdateManifest(
      ManifestFormat::ManifestData& old_manifest,
      const std::vector<std::pair<int, int>>& compact_file_index,
      const std::vector<std::string>& new_files, const int current_level,
      const int insert_index);

  // Read MANIFEST file at kDatabaseDir/kManifestFilename
  Status ReadManifest(ManifestFormat::ManifestData& read_status);

  // Read the Footer of the SST file and store it in the memory
  Status ReadSSTFooter(const std::string& file_abs_path,
                       DataFileFormat::Footer& footer_content);

  // Read the Footer of the SST file and return min/max keys by reference
  Status ReadSSTFooter(const std::string& file_abs_path,
                       DataFileFormat::Footer& footer_content,
                       std::string& min_key, std::string& max_key);

  // Read the IndexBlock of the SST file and store it in the memory
  Status ReadSSTIndex(const std::string& file_abs_path,
                      const DataFileFormat::Footer& footer_content,
                      std::vector<uint32_t>& data_blk_offset);

  Status ReadSSTDataBlock(const std::string& file_abs_path,
                          std::shared_ptr<MemAllocator>& merge_allocator,
                          std::vector<Sequence>& entry_set, const uint64_t size,
                          const ::ssize_t offset,
                          const int reuse_block_id = -1);

  // TODO: Argument?
  // Status WriteSSTFile(const std::vector<const char*> entry_set);

  Status Log(const std::string& msg) { return logger_->Debug(msg); }

 private:
  // Build log and manifest metadata file
  Status BuildMetadataFile();

  // Write manifest content to MANIFEST file at kDatabaseDir/kManifestFilename
  Status WriteManifest(const ManifestFormat::ManifestData& read_status);

  // Write entry_set to specified SST file. Call vector<Sequence> version.
  Status WriteSSTFile(const std::string& file_name,
                      const std::vector<const char*>& entry_set);

  // Write entry_set to specified SST file.
  Status WriteSSTFile(const std::string& file_name,
                      const std::vector<Sequence>& entry_set);

  // Write data blocks to SST file. Called by WriteSSTFile()
  Status WriteSSTData(std::shared_ptr<SequentialWriter>& sw_ptr,
                      const std::vector<Sequence>& entry_set,
                      std::vector<uint32_t>& data_blk_offset,
                      uint32_t& data_block_size);

  // Write data block offset to SST file. Called by WriteSSTFile()
  Status WriteSSTIndex(std::shared_ptr<SequentialWriter>& sw_ptr,
                       const std::vector<uint32_t>& data_blk_offset);

  // Write flexible to SST file. Called by WriteSSTFile()
  // TODO: crc-32?
  Status WriteSSTFlexible(std::shared_ptr<SequentialWriter>& sw_ptr);

  // Deprecated
  Status WriteSSTFileFooter(std::shared_ptr<SequentialWriter>& sw_ptr,
                            const uint32_t max_key_offset,
                            const uint32_t data_block_size,
                            const uint32_t index_block_size,
                            const uint32_t flexible_block_size);

  // Write SST file footer
  Status WriteSSTFileFooter(std::shared_ptr<SequentialWriter>& sw_ptr,
                            const DataFileFormat::Footer& footer);

  // Reader workers for sequential reading
  // std::vector<SequentialReader*> readers_;
  std::vector<std::shared_ptr<SequentialReader>> readers_;

  std::shared_ptr<TCLogger> logger_;

  int file_levels_ = 0;

  // Use file id for .tdb file names.
  // For example, file_id_(0x35AC186F) refers to data file 0000000035AC186F.tdb
  uint64_t file_id_ = 0;

  // Protect readers_ vector from concurrent unsafety
  RAIILock& io_lock_;
};