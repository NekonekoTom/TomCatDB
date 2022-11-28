#include "io_manager.h"

IOManager::IOManager(const std::string& files_dir, RAIILock& io_lock)
    : kDatabaseDir(files_dir), io_lock_(io_lock) {
  // If the kDatabaseDir does not exist or does not have rwx permissions
  if (access(kDatabaseDir.c_str(), F_OK | W_OK | X_OK) != 0) {
    // Assert fails when the directory did not have the desired permissions
    // or mkdir() operation failed
    assert(mkdir(kDatabaseDir.c_str(), 0744) == 0);  // Must have rwx permission
  }

  // Push manifest writer into writers_
  BuildMetadataFile(kManifestFilename);
  // Append log file path at 2nd line
  writers_[0]->AppendToBuffer(
      Sequence("\n" + kDatabaseDir + "/" + kLogFilename));
  writers_[0]->WriteAppendFile();

  // Push log writer into writers_
  BuildMetadataFile(kLogFilename);

  for (int i = 0; i < kDefaultReaderNum; ++i)
    readers_.push_back(new SequentialReader(kDefaultReaderBufferSize));

  // writers_[0]->AppendToBuffer(Sequence("\n00000000.tdb;00000001.tdb"));
  // writers_[0]->AppendToBuffer(Sequence("\n00000002.tdb;00000005.tdb"));
}

IOManager::~IOManager() {
  // DO NOT delete manifest_ or other DBFile ptr here.
  // The mission will be done by deconstructors of the writers.
  // delete manifest_;

  for (auto w : writers_) {
    delete w;
  }
}

Status IOManager::WriteLevel0File(const TCTable* immutable) {
  Status ret;
  // Read from manifest
  auto manifest_data = ReadManifest(ret);

  // If level0 file num reaches kDefaultLevel0FileNum, push level0 to level1
  if (manifest_data.path_to_manifest.empty())
    return Status::FileIOError("Failed to read MANIFEST file.");
  if (!manifest_data.data_files.empty() &&
      manifest_data.data_files[0].size() == kDefaultLevel0FileNum) {
    // TODO: Compaction
    return Status::UndefinedError();
  }

  // Write level0 file
  char file_basename_cstr[17] = {};
  sprintf(file_basename_cstr, "%016lX", file_id_++);
  std::string file_basename(file_basename_cstr);

  ret = WriteSSTFile(
      neko_base::PathJoin(kDatabaseDir, file_basename + kSSTFilePostfix),
      immutable->EntrySet());
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Rewrite manifest file

  return Status::UndefinedError();
}

void IOManager::BuildMetadataFile(const std::string& file_name) {
  // *file = new DBFile(kDatabaseDir + "/" + file_name, DBFile::Mode::kNewFile);
  // assert((*file)->IsOpened());

  BaseWriter* writer =
      new BaseWriter(new DBFile(neko_base::PathJoin(kDatabaseDir, file_name),
                                DBFile::Mode::kNewFile),
                     kDefaultWriterBufferSize);
  writer->AppendToBuffer(neko_base::PathJoin(kDatabaseDir, file_name));
  writer->WriteNewFile();

  writers_.push_back(writer);
}

ManifestFormat::ManifestData IOManager::ReadManifest(Status& read_status) {
  if (readers_.empty()) {
    read_status = Status::FileIOError("No available Reader.");
    return ManifestFormat::ManifestData();
  }

  SequentialReader* reader = readers_.back();
  readers_.pop_back();

  std::string manifest_content;
  read_status =
      reader->ReadEntire(new DBFile(kDatabaseDir + "/" + kManifestFilename,
                                    DBFile::Mode::kReadOnly),
                         manifest_content);
  if (!read_status.StatusNoError()) {
    return ManifestFormat::ManifestData();
  }

  return ManifestFormat::Decode(manifest_content);
}

Status IOManager::WriteSSTFile(const std::string& file_name,
                               const std::vector<Sequence>& entry_set) {
  Status ret;
  std::vector<uint32_t> data_blk_offset;

  // Pre-allocate space for index block.
  data_blk_offset.reserve(DataFileFormat::kApproximateSSTFileSize /
                          DataFileFormat::kDefaultDataBlkSize);

  // BatchWriter* bw = new BatchWriter(
  std::shared_ptr<BatchWriter> bw = std::make_shared<BatchWriter>(
      new DBFile(file_name, DBFile::Mode::kAppend), kDefaultWriterBufferSize);

  // Write entries
  uint32_t data_block_size = 0;
  ret = WriteSSTData(bw, entry_set, data_blk_offset, data_block_size);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write IndexBlock at once
  uint32_t index_block_size =
      (data_blk_offset.size() + 1) * sizeof(data_blk_offset[0]);
  ret = WriteSSTIndex(bw, data_blk_offset);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write FlexibleBlock
  uint32_t flexible_block_size = sizeof(uint32_t);  // TODO: crc-32?
  ret = WriteSSTFlexible(bw);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write Footer
  ret = WriteSSTFileFooter(bw, data_block_size, index_block_size,
                           flexible_block_size);

  return ret;
}

Status IOManager::WriteSSTFile(const std::string& file_name,
                               const std::vector<const char*>& entry_set) {
  std::vector<Sequence> seq_entries;
  for (const char* e : entry_set) {
    seq_entries.push_back(InternalEntry::EntryData(e));
  }
  return WriteSSTFile(file_name, seq_entries);
}

Status IOManager::WriteSSTData(std::shared_ptr<BatchWriter>& bw_ptr,
                               const std::vector<Sequence>& entry_set,
                               std::vector<uint32_t>& data_blk_offset,
                               uint32_t& data_block_size) {
  Status ret;
  uint32_t cur_blk_size = 0;        // Record current block size
  uint32_t cur_offset = 0;          // Record current offset
  uint32_t i = 0, start_point = 0;  // Subscript for iterating the entry_set

  while (i < entry_set.size()) {
    if (cur_blk_size + entry_set[i].size() >
        DataFileFormat::kDefaultDataBlkSize) {
      data_blk_offset.push_back(cur_offset);  // Record block start address
      cur_offset += cur_blk_size;  // Set cur_offset to next block start address
      if (!(ret = bw_ptr->WriteBatch(entry_set, start_point, i + 1))
               .StatusNoError())
        return ret;
      start_point = i + 1;  // Reset start_point
      cur_blk_size = 0;     // Clear block size flag
    } else {
      cur_blk_size += entry_set[i].size();
    }
    ++i;
  }

  // Write last EntryBlock
  if (start_point != i) {
    data_blk_offset.push_back(cur_offset);  // Record block start address
    ret = bw_ptr->WriteBatch(entry_set, start_point, i);

    // Calculate current offset.
    // The following iteration will not cost too much time because the last
    // block size <= kDefaultWriterBufferSize, there won't be many entries.
    for (auto j = start_point; j < i; ++j)
      cur_offset += entry_set[j].size();
  }

  // Return data block size by reference
  data_block_size = cur_offset;

  return ret;
}

Status IOManager::WriteSSTIndex(std::shared_ptr<BatchWriter>& bw_ptr,
                                const std::vector<uint32_t>& data_blk_offset) {
  Status ret;
  uint32_t* index_block = new uint32_t[data_blk_offset.size() + 1];
  uint32_t* index_block_ptr = index_block;

  // First byte in index block is the size of index block
  *index_block_ptr++ = data_blk_offset.size();

  for (auto i = 0; i < data_blk_offset.size(); ++i) {
    *index_block_ptr++ = data_blk_offset[i];
  }
  ret = bw_ptr->WriteFragment(reinterpret_cast<char*>(index_block),
                              (data_blk_offset.size() + 1) * sizeof(uint32_t));
  delete[] index_block;

  return ret;
}

Status IOManager::WriteSSTFlexible(std::shared_ptr<BatchWriter>& bw_ptr) {
  // TODO: crc-32
  return bw_ptr->WriteFragment("TODO", 4);
}

Status IOManager::WriteSSTFileFooter(std::shared_ptr<BatchWriter>& bw_ptr,
                                     const uint32_t data_block_size,
                                     const uint32_t index_block_size,
                                     const uint32_t flexible_block_size) {
  uint32_t footer_size = sizeof(data_block_size) + sizeof(index_block_size) +
                         sizeof(flexible_block_size);
  char* footer = new char[footer_size];
  uint32_t* footer_ptr = reinterpret_cast<uint32_t*>(footer);
  *footer_ptr++ = data_block_size;
  *footer_ptr++ = index_block_size;
  *footer_ptr = flexible_block_size;

  return bw_ptr->WriteFragment(footer, footer_size);
}