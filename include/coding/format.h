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

  // Decode std::string to ManifestData
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
  struct Header {
    // uint32_t file_size;
    uint32_t index_blk_offset;
    uint32_t flexible_blk_offset;
  };

  struct DataBlock {
    ;
  };

  struct IndexBlock {
    uint32_t data_blk_size;
    uint32_t* data_blk_offset;
  };

  // TODO: Unimplemented
  struct FlexibleBlock {
    uint32_t crc;
  };

  static const int kSSTHeaderSize = 8;

  static const int kDefaultDataBlkSize = 4096;

  static const int kApproximateSSTFileSize = 2 << 21;

  DataFileFormat() = delete;
  DataFileFormat(const DataFileFormat&) = delete;
  DataFileFormat& operator=(const DataFileFormat&) = delete;
  ~DataFileFormat() = default;

 private:
};

#endif