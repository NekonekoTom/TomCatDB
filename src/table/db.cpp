#include "db.h"

TCDB::TCDB(const Config& config)
    : global_lock_(mutex_),
      comparator_(std::make_shared<InternalEntryComparator>()),
      volatile_table_(new TCTable(global_lock_, comparator_, 0)),
      io(config.GetConfig("database_dir"), global_lock_) {}

TCDB::~TCDB() {}

const Sequence TCDB::Get(const Sequence& key) {
  // TODO
  return volatile_table_->Get(key);
}

Status TCDB::Insert(const Sequence& key, const Sequence& value) {
  // TODO
  Status ret;

  // TODO: If a compaction process is running, cache current operation
  // and return. The operation will be continued by the end of compaction.

  if (volatile_table_->MemUsage() >= kDefaultSSTFileSize) {
    // WriteLevel0
    global_lock_.Lock();  // Unlock in TransferTable()
    ret = WriteLevel0();  // volatile_table_ now points to a new TCTable

    if (!ret.StatusNoError()) {
      return ret;
    }
  }

  return volatile_table_->Insert(key, value);
}

Status TCDB::Delete(const Sequence& key) {
  // TODO
  return volatile_table_->Delete(key);
}

bool TCDB::ContainsKey(const Sequence& key) {
  // TODO
  return volatile_table_->ContainsKey(key);
}

Status TCDB::TransferTable(const TCTable** immutable) {
  // global_lock_.Lock();
  *immutable = volatile_table_;
  volatile_table_ =
      new TCTable(global_lock_, comparator_, (*immutable)->GetNextEntryID());
  global_lock_.Unlock();  // Locked in Insert()

  return *immutable != nullptr ? Status::NoError() : Status::UndefinedError();
}

Status TCDB::WriteLevel0() {
  Status ret;
  const TCTable* immutable = nullptr;

  if (!TransferTable(&immutable).StatusNoError()) {
    // The volatile_table_ object failed to be transfered
    return Status::UndefinedError();
  }

  // Read from manifest
  ManifestFormat::ManifestData manifest;
  ret = io.ReadManifest(manifest);

  // If level0 file num reaches default level0 file num, push level0 to level1
  if (manifest.path_to_manifest.empty())
    return Status::FileIOError("Failed to read MANIFEST file.");
  if (!manifest.data_files.empty() &&
      manifest.data_files[0].size() >= kDefaultLevelSize[0]) {
    // TODO: Background compaction and rewrite manifest
    return BackgroundCompact(manifest);
  }

  assert(io.WriteLevel0File(immutable, manifest).StatusNoError());

  delete immutable;

  return ret;
}

Status TCDB::BackgroundCompact(ManifestFormat::ManifestData& manifest) {
  Status ret;

  assert(manifest.data_files.size() > 0);

  // Compact the first level 0 SST file by default
  ret = CompactSST(manifest, 0, 0);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // If the current SST files size exceeds limits, start a new Compaction
  for (int i = 0; i < manifest.data_files.size(); ++i) {
    if (manifest.data_files[i].size() >= kDefaultLevelSize[i]) {
      ret = CompactSST(manifest, i, 0);
      if (!ret.StatusNoError()) {
        return ret;
      }
      i = 0;  // Iterate from the beginning
    }
  }

  return ret;
}

Status TCDB::CompactSST(ManifestFormat::ManifestData& manifest,
                        const int current_level, const int compact_file_num) {
  Status ret;

  std::vector<std::pair<int, int>> compact_file_index{
      std::make_pair(current_level, compact_file_num)};  // Index in manifest
  DataFileFormat::Footer footer;
  std::string min_internal_entry, max_internal_entry;

  // Read the specific file's metadata
  ret = io.ReadSSTFooter(
      neko_base::PathJoin(
          io.kDatabaseDir,
          manifest.data_files[current_level][compact_file_num]) +
          io.kSSTFilePostfix,
      footer, min_internal_entry, max_internal_entry);
  if (!ret.StatusNoError()) {
    return ret;
  }

  std::string iter_min_entry, iter_max_entry;
  // Iterate level 0 SST files and find all overlapped SST files
  // Note: if the Compaction starts from above level 0, no need to iterate
  //       level <current_level> files since all SST files are ordered.
  if (current_level == 0) {
    for (int i = 1; i < manifest.data_files[current_level].size(); ++i) {
      if (compact_file_num == i)
        continue;
      ret = io.ReadSSTFooter(
          neko_base::PathJoin(io.kDatabaseDir,
                              manifest.data_files[current_level][i]) +
              io.kSSTFilePostfix,
          footer, iter_min_entry, iter_max_entry);
      if (!ret.StatusNoError()) {
        return ret;
      }

      // If interval (iter_min,iter_max) does not overlap with (min, max)
      if (comparator_->Greater(iter_min_entry, max_internal_entry) ||
          comparator_->Greater(min_internal_entry, iter_max_entry)) {
        continue;
      } else {
        compact_file_index.push_back(std::make_pair(current_level, i));
      }
    }
  }

  // If level <current_level + 1> not empty
  if (current_level + 1 < manifest.data_files.size()) {
    // Iterate level <current_level + 1> SST files. Since all SST files above
    // level 0 are ordered, the search should stop when reaches the max entry.
    for (int i = 0; i < manifest.data_files[current_level + 1].size(); ++i) {
      ret = io.ReadSSTFooter(
          neko_base::PathJoin(io.kDatabaseDir,
                              manifest.data_files[current_level + 1][i]) +
              io.kSSTFilePostfix,
          footer, iter_min_entry, iter_max_entry);
      if (!ret.StatusNoError()) {
        return ret;
      }

      // If interval (iter_min,iter_max) does not overlap with (min, max)
      // if (comparator_->Greater(iter_min_entry, max_internal_entry) ||
      //     comparator_->Greater(min_internal_entry, iter_max_entry)) {
      //   continue;
      // } else {
      //   compact_file_index.push_back(std::make_pair(current_level + 1, i));
      // }
      if (comparator_->Greater(min_internal_entry, iter_max_entry)) {
        // Not reached the range yet, ++i
        continue;
      } else if (comparator_->Greater(iter_min_entry, max_internal_entry)) {
        // Skipped the range [min_internal_entry, max_internal_entry]
        break;
      } else {
        // Interval overlapped
        compact_file_index.push_back(std::make_pair(current_level + 1, i));
      }
    }
  }

  // Found all overlaped SST files, start multi-way merging
  std::vector<std::string> new_files;
  std::vector<std::string> compact_file_abs_path;

  // Make a vector of abs path of the compact files
  for (int i = 0; i < compact_file_index.size(); ++i)
    compact_file_abs_path.push_back(
        neko_base::PathJoin(io.kDatabaseDir,
                            manifest.data_files[compact_file_index[i].first]
                                               [compact_file_index[i].second]) +
        io.kSSTFilePostfix);
  ret = MultiwayMerge(compact_file_abs_path, new_files);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Update Manifest file
  return io.UpdateManifest(manifest, compact_file_index, new_files,
                                    current_level);
  // TODO: A background thread should scan the folder and clean old SST files.
}

Status TCDB::MultiwayMerge(
    const std::vector<std::string>& compact_file_abs_path,
    std::vector<std::string>& new_files) {
  Status ret;

  // TODO: Set max size for the priority_queue to avoid OOM
  std::priority_queue<std::tuple<Sequence, int, int>,
                      std::vector<std::tuple<Sequence, int, int>>,
                      MergeComparator>
      priority_queue;  // The elements of the priority_queue are std::tuples.
                       // The data <Sequence, int, int> can be interpreted as
                       // Sequence: the internal entry data;
                       // first int: the file number, starts from 0;
                       // second int: the Sequence's block_id in MergeAllocator.

  std::vector<int> file_entry_in_queue;  // Record how many entries of file[i]
                                         // are in the priority_queue now
  std::vector<Sequence> entry_set;
  std::vector<DataFileFormat::Footer> footer_list;
  std::vector<std::vector<uint32_t>> index_list;
  std::vector<uint32_t> index_block;
  DataFileFormat::Footer footer;

  // Build a memory pool to store the entries. The MergeAllocator does not need
  // the attribute `default block size` because each time we allocate a block in
  // the pool, the block size is equal to the size of the corresponding SST DataBlock.
  // We hope to manage the memory pool in units of DataBlock.
  std::shared_ptr<MemAllocator> merge_allocator =
      std::make_shared<MergeAllocator>();

  // Initialize merging, read IndexBlocks
  for (int i = 0; i < compact_file_abs_path.size(); ++i) {
    ret = io.ReadSSTFooter(compact_file_abs_path[i], footer);
    if (!ret.StatusNoError())
      return ret;
    footer_list.push_back(footer);

    // Read IndexBlock into the memory
    ret =
        io.ReadSSTIndex(compact_file_abs_path[i], footer, index_block);
    if (!ret.StatusNoError())
      return ret;
    // For conveniently calculating the size of the last block
    index_block.push_back(footer.data_blk_size);
    index_list.push_back(index_block);

    // Read first DataBlock into the memory
    ret = io.ReadSSTDataBlock(compact_file_abs_path[i],
                                       merge_allocator, entry_set,
                                       index_block[1] - index_block[0], 0);
    if (!ret.StatusNoError())
      return ret;

    // Push the DataBlock to the priority queue
    for (auto& e : entry_set) {
      // priority_queue.push(std::make_pair(e, i));
      priority_queue.push(std::make_tuple(e, i, i + 1));
    }
    file_entry_in_queue.push_back(entry_set.size());

    // Clear the buffers
    index_block.clear();
    entry_set.clear();
  }

  // Iterately merge the files, return new file vector by reference
  return IterMerge(compact_file_abs_path, priority_queue, merge_allocator,
                   file_entry_in_queue, index_list, new_files);
}

Status TCDB::IterMerge(
    const std::vector<std::string>& compact_files_abs_path,
    std::priority_queue<std::tuple<Sequence, int, int>,
                        std::vector<std::tuple<Sequence, int, int>>,
                        MergeComparator>& priority_queue,
    std::shared_ptr<MemAllocator>& merge_allocator,
    std::vector<int>& file_entry_in_queue,
    const std::vector<std::vector<uint32_t>>& index_list,
    std::vector<std::string>& new_files) {
  Status ret;

  std::vector<int> index_block_ptr(index_list.size(), 1);  // IndexBlock ptrs
  std::vector<std::tuple<Sequence, int, int>> output_buffer;
  output_buffer.reserve(kDefaultSSTFileSize);
  int current_sst_size = 0;

  while (!priority_queue.empty()) {
    auto pq_item = priority_queue.top();  // std::tuple<Sequence, int, int>

    current_sst_size += std::get<0>(pq_item).size();
    if (current_sst_size >= kDefaultSSTFileSize) {
      std::string file_basename;
      ret = io.WriteMergeSSTFile(
          output_buffer, file_basename,
          merge_allocator);  // TODO: Unref from the mem pool Here.
      if (!ret.StatusNoError()) {
        return ret;
      }
      new_files.push_back(std::move(file_basename));
      // Release memory resources
      merge_allocator->ReleaseIdleSpace();
      output_buffer.clear();  // TODO: Time cost?
      current_sst_size = 0;
      continue;
    }
    priority_queue.pop();
    output_buffer.push_back(pq_item);

    // If a DataBlock read has finished, read the next DataBlock
    if (--file_entry_in_queue[std::get<1>(pq_item)] == 0) {
      int current_file_num = std::get<1>(pq_item);

      // If there are no more SSTDataBlocks in the current_file, continue;
      if (index_block_ptr[current_file_num] >=
          index_list[current_file_num].size() - 1) {
        continue;
      }

      int block_size =
          index_list[current_file_num][index_block_ptr[current_file_num] + 1] -
          index_list[current_file_num][index_block_ptr[current_file_num]];
      int block_offset =
          index_list[current_file_num][index_block_ptr[current_file_num]];
      std::vector<Sequence> entry_set;

      ret = io.ReadSSTDataBlock(
          compact_files_abs_path[current_file_num], merge_allocator, entry_set,
          block_size, block_offset);  // Do not reuse block
      if (!ret.StatusNoError()) {
        return ret;
      }

      ++index_block_ptr[current_file_num];  // Move index_block pointer
      file_entry_in_queue[current_file_num] += entry_set.size();

      for (auto& e : entry_set) {
        priority_queue.push(std::make_tuple(e, current_file_num,
                                            merge_allocator->BlockCount() - 1));
      }
    }
  }

  // If output_buffer not empty, flush the last entries to a SST file even if
  // the new SST file does not reach the kDefaultSSTFileSize limit.
  if (!output_buffer.empty()) {
    std::string file_basename;
    ret = io.WriteMergeSSTFile(
        output_buffer, file_basename,
        merge_allocator);  // Unref from the mem pool
    if (!ret.StatusNoError()) {
      return ret;
    }
    new_files.push_back(std::move(file_basename));
  }

  return ret;
}