#ifndef FORMAT_H_
#define FORMAT_H_

#include "tools.h"

class FileFormat {
 public:
  FileFormat() = delete;
  FileFormat(const FileFormat&) = delete;
  FileFormat& operator=(const FileFormat&) = delete;
  ~FileFormat() = default;

 private:
};

// Manifest file format as:
// L1: path of the manifest file
// L2: path of the log file
// L3: level 0     file names, seperated by ";"
// Li: level (i-3) file names, seperated by ";"
class ManifestFormat : public FileFormat {
 public:
  struct ManifestData {
    std::string path_to_manifest;
    std::string path_to_log;
    std::vector<std::vector<std::string>> data_files;
  };

  ManifestFormat() = delete;
  ManifestFormat(const ManifestFormat&) = delete;
  ManifestFormat& operator=(const ManifestFormat&) = delete;
  ~ManifestFormat() = default;

  // Decode std::string to ManifestData. DO NOT check legality.
  static ManifestData Decode(const std::string& manifest);

  // TODO
  static void Encode();

 private:
};

// TODO
// Log file format as:
// L1: path of the log file
// Li: log entries started by uint64_t head(8B)
class LogFormat : public FileFormat {
 public:
  LogFormat() = delete;
  LogFormat(const LogFormat&) = delete;
  LogFormat& operator=(const LogFormat&) = delete;
  ~LogFormat() = default;

 private:
};

class DataFileFormat : public FileFormat {
 public:
  struct DataBlock {
    ;
  };

  struct IndexBlock {
    uint32_t data_blk_count;
    uint32_t* data_blk_offset;
  };

  // TODO: Unimplemented
  struct FlexibleBlock {
    uint32_t crc;
  };

  struct Footer {
    Footer() = default;
    explicit Footer(const char* footer_str) {
      min_key_size = *reinterpret_cast<const uint32_t*>(footer_str);
      max_key_offset = *reinterpret_cast<const uint32_t*>(footer_str + 4);
      max_key_size = *reinterpret_cast<const uint32_t*>(footer_str + 8);
      data_blk_size = *reinterpret_cast<const uint32_t*>(footer_str + 12);
      index_blk_size = *reinterpret_cast<const uint32_t*>(footer_str + 16);
      flexible_blk_size = *reinterpret_cast<const uint32_t*>(footer_str + 20);
    }
    Footer(const uint32_t minks, const uint32_t maxko, const uint32_t maxks,
           const uint32_t dbs, const uint32_t ibs, const uint32_t fbs)
        : min_key_size(minks),
          max_key_offset(maxko),
          max_key_size(maxks),
          data_blk_size(dbs),
          index_blk_size(ibs),
          flexible_blk_size(fbs) {}

    // uint32_t min_key_offset = 0;
    uint32_t min_key_size;
    uint32_t max_key_offset;
    uint32_t max_key_size;
    uint32_t data_blk_size;
    uint32_t index_blk_size;
    uint32_t flexible_blk_size;
  };

  static const int kSSTFooterSize = 6 * sizeof(uint32_t);

  // When the current data block size > kDefaultDataBlkSize, the writer
  // writes the current data block into the SST file at once.
  // The real size of a data block can be calculated by the start address
  // of the next block minus the start address of the current block.
  static const int kDefaultDataBlkSize = 4096;

  static const int kApproximateSSTFileSize = 1 << 21;

  DataFileFormat() = delete;
  DataFileFormat(const DataFileFormat&) = delete;
  DataFileFormat& operator=(const DataFileFormat&) = delete;
  ~DataFileFormat() = default;

 private:
};

#endif