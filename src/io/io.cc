#include "io.h"

TCIO::TCIO(const std::string& files_dir, RAIILock& io_lock)
    : kDatabaseDir(files_dir), io_lock_(io_lock) {
  // If the kDatabaseDir does not exist or does not have rwx permissions
  int stat = access(kDatabaseDir.c_str(), F_OK | W_OK | X_OK);
  while (stat != 0) {
    while (stat != 0)
      stat = mkdir(kDatabaseDir.c_str(), 0744);
    stat = access(kDatabaseDir.c_str(), F_OK | W_OK | X_OK);
    if (stat != 0)
      printf("errno: %d\n", errno);
  }

  Status ret = BuildMetadataFile();
  assert(ret.StatusNoError());

  logger_ = std::make_shared<TCLogger>(
      neko_base::PathJoin(kDatabaseDir, kLogFilename));

  for (int i = 0; i < kDefaultReaderNum; ++i)
    readers_.push_back(
        std::make_shared<SequentialReader>(kDefaultReaderBufferSize));
}

TCIO::~TCIO() {
  // DO NOT delete manifest_ or other DBFile ptr here.
  // The mission will be done by deconstructors of the writers.
  // delete manifest_;

  // Use shared_ptr instead of raw pointers for Reader and Writer,
  // don't need to delete.
}

Status TCIO::WriteLevel0File(const TCTable* immutable,
                             ManifestFormat::ManifestData& manifest) {
  Status ret;

  // Write level0 file
  char file_basename_cstr[17] = {};
  io_lock_.Lock();  // Protect the file_id_
  sprintf(file_basename_cstr, "%016lX", file_id_++);
  io_lock_.Unlock();
  std::string file_basename(file_basename_cstr);

  ret = WriteSSTFile(
      neko_base::PathJoin(kDatabaseDir, file_basename + kSSTFilePostfix),
      immutable->EntrySet());
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Rewrite manifest file
  if (manifest.data_files.empty())
    manifest.data_files.push_back(std::vector<std::string>());
  manifest.data_files[0].push_back(file_basename);

  return WriteManifest(manifest);
}

Status TCIO::WriteNewSSTFile(const std::vector<Sequence>& entry_set,
                             std::string& file_basename) {
  Status ret;

  char file_basename_cstr[17] = {};
  io_lock_.Lock();  // Protect the file_id_
  sprintf(file_basename_cstr, "%016lX", file_id_++);
  io_lock_.Unlock();

  file_basename = std::string(file_basename_cstr);

  ret = WriteSSTFile(
      neko_base::PathJoin(kDatabaseDir, file_basename + kSSTFilePostfix),
      entry_set);

  return ret;
}

Status TCIO::WriteMergeSSTFile(
    const std::vector<std::tuple<Sequence, int, int>>& item_set,
    std::string& file_basename,
    std::shared_ptr<MemAllocator>& merge_allocator) {
  std::vector<Sequence> entry_set;
  entry_set.reserve(item_set.size());

  for (auto& i : item_set) {
    merge_allocator->Unref(std::get<2>(i));
    entry_set.push_back(std::get<0>(i));
  }

  Status ret = WriteNewSSTFile(entry_set, file_basename);

  // Clear merge_allocator
  merge_allocator->ReleaseIdleSpace();

  return ret;
}

Status TCIO::UpdateManifest(
    ManifestFormat::ManifestData& old_manifest,
    const std::vector<std::pair<int, int>>& compact_file_index,
    const std::vector<std::string>& new_files, const int current_level,
    const int insert_index) {
  Status ret;

  ManifestFormat::ManifestData manifest = old_manifest;

  // Delete the compacted files from the manifest
  // Clean current level
  auto it = compact_file_index.begin();
  std::vector<std::vector<std::string>::size_type> remove_points;
  while (it != compact_file_index.end() && it->first == current_level) {
    // TODO: Time cost!
    remove_points.push_back(it->second);
    ++it;
  }
  // The std::vector remove_points MUST be ascending
  neko_base::Remove(manifest.data_files[current_level], remove_points);

  // Insert new SST files
  if (manifest.data_files.size() <= current_level + 1) {
    // Push an empty vector
    manifest.data_files.push_back(std::vector<std::string>());
  }

  std::pair<int, int> boundary;
  if (it == compact_file_index.end()) {
    // Did not compact any files at level <current_level + 1>

    // The insert_index has 3 conditions:
    //   0: push front;
    //   data_files.size(): push back;
    //   others: insert at insert_index;
    // Here, the insert_index can only be 0 or data_files.size()
    assert(insert_index == 0 ||
           insert_index == manifest.data_files[current_level + 1].size());
    boundary = std::make_pair(insert_index, insert_index);
  } else {
    boundary = std::make_pair(it->second, compact_file_index.back().second + 1);
  }

  // Note: compact_file_index at level <current_level + 1> are consistent.
  if (!neko_base::RearrangeFilesInManifest(
          manifest.data_files[current_level + 1], boundary, new_files,
          current_level + 1))
    return Status::UndefinedError();

  ret = WriteManifest(manifest);
  if (ret.StatusNoError()) {
    old_manifest = manifest;
  }
  return ret;
}

Status TCIO::ReadManifest(ManifestFormat::ManifestData& manifest) {
  Status ret;

  io_lock_.Lock();  // TODO: Blocking queue?
  if (readers_.empty()) {
    io_lock_.Unlock();
    return Status::FileIOError("No available Reader.");
  }

  std::shared_ptr<SequentialReader> reader = readers_.back();
  readers_.pop_back();
  io_lock_.Unlock();

  std::string manifest_content;
  ret = reader->ReadEntire(
      new DBFile(neko_base::PathJoin(kDatabaseDir, kManifestFilename),
                 DBFile::Mode::kReadOnly),
      manifest_content);
  if (ret.StatusNoError()) {
    manifest = ManifestFormat::Decode(manifest_content);
  }

  // Push the SequentialReader object back to reader_ before return
  io_lock_.Lock();
  readers_.push_back(reader);
  io_lock_.Unlock();

  return ret;
}

// TODO: Reuse another version
Status TCIO::ReadSSTFooter(const std::string& file_abs_path,
                           DataFileFormat::Footer& footer) {
  Status ret;

  // Get a SequentialReader
  io_lock_.Lock();
  auto footer_reader = readers_.back();
  readers_.pop_back();
  io_lock_.Unlock();

  // Get file size
  auto size = footer_reader->FlieSize(file_abs_path.c_str());

  // Read Footer
  std::string footer_content;
  ret = footer_reader->Read(new DBFile(file_abs_path), footer_content,
                            DataFileFormat::kSSTFooterSize,
                            size - DataFileFormat::kSSTFooterSize);
  if (!ret.StatusNoError())
    return ret;

  // Return values
  footer = DataFileFormat::Footer(footer_content.c_str());

  io_lock_.Lock();
  readers_.push_back(footer_reader);
  io_lock_.Unlock();

  return Status::NoError();
}

Status TCIO::ReadSSTFooter(const std::string& file_abs_path,
                           DataFileFormat::Footer& footer, std::string& min_key,
                           std::string& max_key) {
  Status ret;

  // Get a SequentialReader
  io_lock_.Lock();
  auto footer_reader = readers_.back();
  readers_.pop_back();
  io_lock_.Unlock();

  // Get file size
  auto size = footer_reader->FlieSize(file_abs_path.c_str());

  // Read Footer
  std::string footer_content;
  ret = footer_reader->Read(new DBFile(file_abs_path), footer_content,
                            DataFileFormat::kSSTFooterSize,
                            size - DataFileFormat::kSSTFooterSize);
  if (!ret.StatusNoError())
    return ret;

  // Return values
  footer = DataFileFormat::Footer(footer_content.c_str());

  ret = footer_reader->Read(new DBFile(file_abs_path), min_key,
                            footer.min_key_size, 0);
  if (!ret.StatusNoError())
    return ret;
  ret = footer_reader->Read(new DBFile(file_abs_path), max_key,
                            footer.max_key_size, footer.max_key_offset);
  if (!ret.StatusNoError())
    return ret;

  io_lock_.Lock();
  readers_.push_back(footer_reader);
  io_lock_.Unlock();

  return Status::NoError();
}

Status TCIO::ReadSSTFooter(
    const std::vector<std::string>& file_abs_path,
    std::vector<DataFileFormat::Footer>& footer_contents,
    std::vector<std::pair<std::string, std::string>>& min_max_keys) {
  Status ret;

  DataFileFormat::Footer footer;
  std::string min_key, max_key;
  for (auto& f_abs_path : file_abs_path) {
    ret = ReadSSTFooter(f_abs_path, footer, min_key, max_key);
    if (!ret.StatusNoError())
      return ret;

    footer_contents.push_back(footer);
    min_max_keys.emplace_back(min_key, max_key);
  }

  return ret;
}

Status TCIO::ReadSSTBoundary(const std::string& file_abs_path,
                             std::string& min_key, std::string& max_key) {
  Status ret;

  DataFileFormat::Footer footer;

  return ReadSSTFooter(file_abs_path, footer, min_key, max_key);
}

Status TCIO::ReadSSTGroupBoundary(
    const std::vector<std::string>& file_abs_path,
    std::vector<std::pair<std::string, std::string>>& min_max_keys) {
  Status ret;

  DataFileFormat::Footer footer;
  std::string min_key, max_key;
  for (auto& f_abs_path : file_abs_path) {
    ret = ReadSSTFooter(f_abs_path, footer, min_key, max_key);
    if (!ret.StatusNoError())
      return ret;

    min_max_keys.emplace_back(min_key, max_key);
  }

  return ret;
}

Status TCIO::ReadSSTIndex(const std::string& file_abs_path,
                          const DataFileFormat::Footer& footer,
                          std::vector<uint32_t>& data_blk_offset) {
  Status ret;

  // Get a SequentialReader
  io_lock_.Lock();
  auto index_reader = readers_.back();
  readers_.pop_back();
  io_lock_.Unlock();

  std::string index_content;
  ret = index_reader->Read(new DBFile(file_abs_path), index_content,
                           footer.index_blk_size, footer.data_blk_size);
  if (!ret.StatusNoError())
    return ret;

  const uint32_t* index_ptr =
      reinterpret_cast<const uint32_t*>(index_content.c_str());
  uint32_t data_blk_count = *index_ptr;

  ++index_ptr;
  for (int i = 0; i < data_blk_count; ++i) {
    data_blk_offset.push_back(*index_ptr++);
  }

  io_lock_.Lock();
  readers_.push_back(index_reader);
  io_lock_.Unlock();

  return ret;
}

Status TCIO::ReadSSTDataBlock(const std::string& file_abs_path,
                              std::shared_ptr<MemAllocator>& merge_allocator,
                              std::vector<Sequence>& entry_set,
                              const uint64_t block_size,
                              const ::ssize_t block_offset,
                              const int reuse_block_id) {
  Status ret;

  // Get a SequentialReader
  io_lock_.Lock();
  auto data_reader = readers_.back();
  readers_.pop_back();
  io_lock_.Unlock();

  // Read DataBlock
  std::string data_content;
  ret = data_reader->Read(new DBFile(file_abs_path), data_content, block_size,
                          block_offset);
  if (!ret.StatusNoError())
    return ret;

  // Copy the content of the DataBlock from the stack to the MemAllocator
  char* data_block = nullptr;
  if (reuse_block_id == -1) {
    // Do not reallocate memory
    data_block = merge_allocator->Allocate(data_content.size());
  } else {
    data_block =
        merge_allocator->Reallocate(data_content.size(), reuse_block_id);
  }
  if (!data_block)
    return Status::FileIOError(
        "Memory in MemAllocator/MergeAllocator not allocated.");
  std::memcpy(data_block, data_content.c_str(), data_content.size());

  auto data_offset = static_cast<std::string::size_type>(0);
  while (data_offset < data_content.size()) {
    entry_set.push_back(InternalEntry::EntryData(data_block + data_offset));
    data_offset += entry_set.back().size();
  }

  // Update reference counter at once
  if (reuse_block_id == -1) {
    merge_allocator->RefLast(entry_set.size() - 1);
  } else {
    merge_allocator->RefBlock(entry_set.size() - 1, reuse_block_id);
  }

  io_lock_.Lock();
  readers_.push_back(data_reader);
  io_lock_.Unlock();

  return ret;
}

Status TCIO::BuildMetadataFile() {
  Status ret = Status().NoError();
  std::shared_ptr<SequentialWriter> writer;

  // If the manifest file does not exist or does not have rwx permissions
  if (access(neko_base::PathJoin(kDatabaseDir, kManifestFilename).c_str(),
             F_OK) != 0) {
    // Write manifest
    writer = std::make_shared<SequentialWriter>(
        new DBFile(neko_base::PathJoin(kDatabaseDir, kManifestFilename),
                   DBFile::Mode::kNewFile),
        kDefaultWriterBufferSize);
    ret = writer->WriteFragment(
        neko_base::PathJoin(kDatabaseDir, kManifestFilename) + "\n" +
        neko_base::PathJoin(kDatabaseDir, kLogFilename));
    if (!ret.StatusNoError())
      return ret;
  }

  if (access(neko_base::PathJoin(kDatabaseDir, kLogFilename).c_str(), F_OK) !=
      0) {
    // Write log
    writer = std::make_shared<SequentialWriter>(
        new DBFile(neko_base::PathJoin(kDatabaseDir, kLogFilename),
                   DBFile::Mode::kNewFile),
        kDefaultWriterBufferSize);
    ret = writer->WriteFragment("Created new database.\n");
    // writer->WriteFragment(neko_base::PathJoin(kDatabaseDir, kLogFilename));
  }

  return ret;
}

Status TCIO::WriteManifest(const ManifestFormat::ManifestData& manifest) {
  // TODO: File lock?

  Status ret;
  std::shared_ptr<SequentialWriter> sw = std::make_shared<SequentialWriter>(
      new DBFile(neko_base::PathJoin(kDatabaseDir, kManifestFilename),
                 DBFile::Mode::kNewFile),
      kDefaultWriterBufferSize);

  ret = sw->WriteFragment(manifest.path_to_manifest + "\n");
  if (!ret.StatusNoError())
    return ret;

  ret = sw->WriteFragment(manifest.path_to_log + "\n");
  if (!ret.StatusNoError())
    return ret;

  for (int level = 0; level < manifest.data_files.size(); ++level) {
    std::string line;
    for (auto f : manifest.data_files[level]) {
      line += f + ';';
    }
    if (line.empty())
      line.push_back('\n');
    else
      line.back() = '\n';
    ret = sw->WriteFragment(line);
  }

  return ret;
}

Status TCIO::WriteSSTFile(const std::string& file_name,
                          const std::vector<Sequence>& entry_set) {
  Status ret;
  std::vector<uint32_t> data_blk_offset;

  // Pre-allocate space for index block
  data_blk_offset.reserve(DataFileFormat::kApproximateSSTFileSize /
                          DataFileFormat::kDefaultDataBlkSize);

  std::shared_ptr<SequentialWriter> sw = std::make_shared<SequentialWriter>(
      new DBFile(file_name, DBFile::Mode::kAppend), kDefaultWriterBufferSize);

  // Write entries
  uint32_t data_block_size = 0;
  ret = WriteSSTData(sw, entry_set, data_blk_offset, data_block_size);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write IndexBlock at once
  uint32_t index_block_size = (data_blk_offset.size() + 1) * sizeof(uint32_t);
  ret = WriteSSTIndex(sw, data_blk_offset);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write FlexibleBlock
  uint32_t flexible_block_size = sizeof(uint32_t);  // TODO: crc-32?
  ret = WriteSSTFlexible(sw);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Write Footer
  ret = WriteSSTFileFooter(
      sw, DataFileFormat::Footer(entry_set.front().size(),
                                 data_block_size - entry_set.back().size(),
                                 entry_set.back().size(), data_block_size,
                                 index_block_size, flexible_block_size));

  return ret;
}

Status TCIO::WriteSSTFile(const std::string& file_name,
                          const std::vector<const char*>& entry_set) {
  std::vector<Sequence> seq_entries;
  for (const char* e : entry_set) {
    seq_entries.push_back(InternalEntry::EntryData(e));
  }
  return WriteSSTFile(file_name, seq_entries);
}

Status TCIO::WriteSSTData(std::shared_ptr<SequentialWriter>& sw_ptr,
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

      // Set cur_offset to next block start address
      cur_offset += cur_blk_size + entry_set[i].size();

      if (!(ret = sw_ptr->WriteBatch(entry_set, start_point, i + 1))
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
    ret = sw_ptr->WriteBatch(entry_set, start_point, i);

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

Status TCIO::WriteSSTIndex(std::shared_ptr<SequentialWriter>& sw_ptr,
                           const std::vector<uint32_t>& data_blk_offset) {
  Status ret;
  uint32_t* index_block = new uint32_t[data_blk_offset.size() + 1];
  uint32_t* index_block_ptr = index_block;

  // First byte in index block is the size of index block
  *index_block_ptr++ = data_blk_offset.size();

  for (auto i = 0; i < data_blk_offset.size(); ++i) {
    *index_block_ptr++ = data_blk_offset[i];
  }
  ret = sw_ptr->WriteFragment(reinterpret_cast<char*>(index_block),
                              (data_blk_offset.size() + 1) * sizeof(uint32_t));
  delete[] index_block;

  return ret;
}

Status TCIO::WriteSSTFlexible(std::shared_ptr<SequentialWriter>& sw_ptr) {
  // TODO: crc-32
  return sw_ptr->WriteFragment("TODO", 4);
}

Status TCIO::WriteSSTFileFooter(std::shared_ptr<SequentialWriter>& sw_ptr,
                                const DataFileFormat::Footer& footer) {
  char* footer_cptr = new char[DataFileFormat::kSSTFooterSize];
  uint32_t* footer_ptr = reinterpret_cast<uint32_t*>(footer_cptr);
  *footer_ptr++ = footer.min_key_size;
  *footer_ptr++ = footer.max_key_offset;
  *footer_ptr++ = footer.max_key_size;
  *footer_ptr++ = footer.data_blk_size;
  *footer_ptr++ = footer.index_blk_size;
  *footer_ptr = footer.flexible_blk_size;

  Status ret =
      sw_ptr->WriteFragment(footer_cptr, DataFileFormat::kSSTFooterSize);
  delete[] footer_cptr;
  return ret;
}
