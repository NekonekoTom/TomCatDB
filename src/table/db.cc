#include "db.h"

TCDB::TCDB(const Config& config)
    : global_lock_(mutex_),
      comparator_(std::make_shared<InternalEntryComparator>()),
      volatile_table_(new TCTable(global_lock_, comparator_, 0)),
      io_(config.GetConfig("database_dir"), global_lock_),
      query_buffer(std::make_shared<QueryAllocator>()) {
  // TODO: If the database already exists, read manifest and update file_id_
  //       and entry_id_.

  cache = std::make_shared<LRUCache<Sequence, Sequence, SeqHash, SeqEqual>>(12);
}

TCDB::~TCDB() {}

std::string TCDB::Get(const Sequence& key) {
  // TODO

  // // Make a virtual entry in the cache.
  // // If the entry does not exist, evict it later
  // // TODO: Evict
  // // TODO: Allocate on LRUCache and MemAllocator

  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;
  char* internal_entry = query_buffer->Allocate(entry_size);

  // The value, ID, and op_type are invalid
  Status enc = InternalEntry::EncodeInternal(
      key, Sequence(), static_cast<uint64_t>(0) - 1, InternalEntry::kDelete,
      internal_entry);
  if (!enc.StatusNoError())
    return std::string();

  // Try to find the entry in volatile_table_
  // Sequence result = volatile_table_->Get(key);
  Sequence result = volatile_table_->Get(internal_entry);
  if (result.size() != 0) {
    return std::string(result.data(), result.size());
  }

  // Not found in the memory, search in the SST files
  ManifestFormat::ManifestData manifest;
  Status ret;
  ret = io_.ReadManifest(manifest);
  if (!ret.StatusNoError()) {
    // Log the error message
    return std::string();
  }

  int level = 0;
  Sequence query_key(internal_entry, entry_size);
  while (result.size() == 0 && level < manifest.data_files.size()) {
    // ret = SearchLevel(key, result, manifest, level++);
    ret = SearchLevel(query_key, result, manifest, level++);
  }

  result = InternalEntry::EntryValue(result.data());
  return std::string(result.data(), result.size());
}

Status TCDB::Insert(const Sequence& key, const Sequence& value) {
  // TODO
  Status ret;

  // TODO: MVCC

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
  ret = io_.ReadManifest(manifest);

  // If level0 file num reaches default level0 file num, push level0 to level1
  if (manifest.path_to_manifest.empty())
    return Status::FileIOError("Failed to read MANIFEST file.");
  if (!manifest.data_files.empty() &&
      manifest.data_files[0].size() >= kDefaultLevelSize[0]) {
    // TODO: Background compaction and rewrite manifest
    ret = BackgroundCompact(manifest);
    if (!ret.StatusNoError())
      return ret;
  }

  assert(io_.WriteLevel0File(immutable, manifest).StatusNoError());

  delete immutable;

  return ret;
}

Status TCDB::BackgroundCompact(ManifestFormat::ManifestData& manifest) {
  Status ret;

  assert(manifest.data_files.size() > 0);
  io_.Log("Starting BackgroundCompact.");

  // Compact the first level 0 SST file by default
  ret = CompactSST(manifest, 0, 0);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // If the current SST files size exceeds limits, start a new Compaction
  for (int i = 0; i < manifest.data_files.size();) {
    if (manifest.data_files[i].size() >= kDefaultLevelSize[i]) {
      ret = CompactSST(manifest, i, 0);  // TODO: Each compaction should compact
                                         // all files that exceed the limit.
      if (!ret.StatusNoError()) {
        return ret;
      }
      // i = 0;  // Iterate from the beginning
    } else {
      ++i;
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
  ret = io_.ReadSSTFooter(
      neko_base::PathJoin(
          io_.kDatabaseDir,
          manifest.data_files[current_level][compact_file_num]) +
          io_.kSSTFilePostfix,
      footer, min_internal_entry, max_internal_entry);
  if (!ret.StatusNoError()) {
    return ret;
  }

  std::string iter_min_entry, iter_max_entry;
  // Iterate level 0 SST files and find all overlapped SST files
  // Note: if the Compaction starts from above level 0, no need to iterate
  //       level <current_level> files since all SST files are ordered.
  if (current_level == 0) {
    for (int i = 0; i < manifest.data_files[current_level].size(); ++i) {
      if (compact_file_num == i)
        continue;
      ret = io_.ReadSSTFooter(
          neko_base::PathJoin(io_.kDatabaseDir,
                              manifest.data_files[current_level][i]) +
              io_.kSSTFilePostfix,
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

  // Record where the new files will be inserted
  // auto file_num = manifest.data_files[current_level + 1].size();
  auto file_num = 0;
  int insert_from_index = file_num;

  // If level <current_level + 1> not empty
  if (current_level + 1 < manifest.data_files.size()) {
    file_num = manifest.data_files[current_level + 1].size();
    insert_from_index = file_num;
    // Iterate level <current_level + 1> SST files. Since all SST files above
    // level 0 are ordered, the search should stop when reaches the max entry.
    for (int i = 0; i < manifest.data_files[current_level + 1].size(); ++i) {
      ret = io_.ReadSSTFooter(
          neko_base::PathJoin(io_.kDatabaseDir,
                              manifest.data_files[current_level + 1][i]) +
              io_.kSSTFilePostfix,
          footer, iter_min_entry, iter_max_entry);
      if (!ret.StatusNoError()) {
        return ret;
      }

      if (comparator_->Greater(min_internal_entry, iter_max_entry)) {
        // Not reached the range yet, ++i
        continue;
      } else if (comparator_->Greater(iter_min_entry, max_internal_entry)) {
        // Skipped the range [min_internal_entry, max_internal_entry]
        if (i == 0)  // All existed files are larger than the new file
          insert_from_index = 0;
        break;
      } else {
        if (insert_from_index == file_num)  // sfsi not set
          insert_from_index = i;
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
        neko_base::PathJoin(io_.kDatabaseDir,
                            manifest.data_files[compact_file_index[i].first]
                                               [compact_file_index[i].second]) +
        io_.kSSTFilePostfix);
  ret = MultiwayMerge(compact_file_abs_path, new_files);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // Update Manifest file
  return io_.UpdateManifest(manifest, compact_file_index, new_files,
                            current_level, insert_from_index);
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
  // std::vector<DataFileFormat::Footer> footer_list;
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
    ret = io_.ReadSSTFooter(compact_file_abs_path[i], footer);
    if (!ret.StatusNoError())
      return ret;
    // footer_list.push_back(footer);

    // Read IndexBlock into the memory
    ret = io_.ReadSSTIndex(compact_file_abs_path[i], footer, index_block);
    if (!ret.StatusNoError())
      return ret;
    // For conveniently calculating the size of the last block
    index_block.push_back(footer.data_blk_size);
    index_list.push_back(index_block);

    // Read first DataBlock into the memory
    ret = io_.ReadSSTDataBlock(compact_file_abs_path[i], merge_allocator,
                               entry_set, index_block[1] - index_block[0], 0);
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
      ret = io_.WriteMergeSSTFile(
          output_buffer, file_basename,
          merge_allocator);  // Unref from the mem pool Here.
      if (!ret.StatusNoError()) {
        return ret;
      }
      new_files.push_back(std::move(file_basename));
      // Release memory resources
      merge_allocator->ReleaseIdleSpace();
      output_buffer.clear();
      current_sst_size = 0;
      continue;
    }
    priority_queue.pop();

    // TODO: Compact kDelete
    // Compact entries with the same key.
    // Since all entries are in ascending order, we keep the last one only.
    if (!output_buffer.empty() &&
        comparator_->Equal(std::get<0>(output_buffer.back()).data(),
                           std::get<0>(pq_item).data())) {
      // Current entry has the same key as the previous one. Replace the last
      // entry in the output_buffer with the current entry because the current
      // one has a larger id.
      merge_allocator->Unref(std::get<2>(output_buffer.back()));  // Unref
      output_buffer.back() = pq_item;
    } else {
      output_buffer.push_back(pq_item);
    }

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

      ret = io_.ReadSSTDataBlock(compact_files_abs_path[current_file_num],
                                 merge_allocator, entry_set, block_size,
                                 block_offset);  // Do not reuse block
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

  // TODO: Compact redundant entries.
  // If output_buffer not empty, flush the last entries to a SST file even if
  // the new SST file does not reach the kDefaultSSTFileSize limit.
  if (!output_buffer.empty()) {
    std::string file_basename;
    ret = io_.WriteMergeSSTFile(output_buffer, file_basename,
                                merge_allocator);  // Unref from the mem pool
    if (!ret.StatusNoError()) {
      return ret;
    }
    new_files.push_back(std::move(file_basename));
  }

  return ret;
}

Status TCDB::SearchLevel(const Sequence& query_key, Sequence& ret_entry,
                         const ManifestFormat::ManifestData& manifest,
                         const int level) {
  Status ret;

  assert(level < manifest.data_files.size());

  std::string iter_min_entry, iter_max_entry;  // Boundaries of an SST file
  std::string file_abs_path;
  std::string prev_ret_key;  // The expected key may scatter in different files,
                             // prev_ret_key denotes the largest one in previous
                             //  SST files.
  DataFileFormat::Footer footer;

  std::shared_ptr<QueryComparator> comp = std::make_shared<QueryComparator>();
  if (level == 0) {
    // Search all level 0 files
    for (int i = 0; i < manifest.data_files[level].size(); ++i) {
      file_abs_path =
          neko_base::PathJoin(io_.kDatabaseDir, manifest.data_files[level][i]) +
          io_.kSSTFilePostfix;
      ret = io_.ReadSSTFooter(file_abs_path, footer, iter_min_entry,
                              iter_max_entry);
      if (!ret.StatusNoError())
        return ret;

      // If the query_key is NOT in the range of [iter_min,iter_max]
      if (comp->LessOrEquals(query_key.data(), iter_max_entry.c_str()) &&
          comp->GreaterOrEquals(query_key.data(), iter_min_entry.c_str())) {
        // The query_key may be in the SST file
        return GetFromSST(query_key, ret_entry, file_abs_path, footer);
      }  // Else not in the range, continue.
    }
  } else {
    // Since all SST files at level 1+ don't overlap, the search should stop
    // when iter_min_entry > query_key
    for (int i = 0; i < manifest.data_files[level].size(); ++i) {
      file_abs_path =
          neko_base::PathJoin(io_.kDatabaseDir, manifest.data_files[level][i]) +
          io_.kSSTFilePostfix;
      ret = io_.ReadSSTFooter(file_abs_path, footer, iter_min_entry,
                              iter_max_entry);
      if (!ret.StatusNoError())
        return ret;

      if (comp->LessOrEquals(query_key.data(), iter_max_entry.c_str()) &&
          comp->GreaterOrEquals(query_key.data(), iter_min_entry.c_str())) {
        // The key may be in the range of [iter_min_entry, iter_max_entry]
        return GetFromSST(query_key, ret_entry, file_abs_path, footer);
      }  // Else not in the range, continue.
    }
  }

  return ret;
}

Status TCDB::GetFromSST(const Sequence& query_key, Sequence& ret_entry,
                        const std::string& file_abs_path,
                        const DataFileFormat::Footer& footer) {
  Status ret;

  std::vector<uint32_t> index_block;
  ret = io_.ReadSSTIndex(file_abs_path, footer, index_block);
  if (!ret.StatusNoError())
    return ret;
  // For conveniently calculating the size of the last block
  index_block.push_back(footer.data_blk_size);

  std::shared_ptr<QueryComparator> comp = std::make_shared<QueryComparator>();
  std::shared_ptr<MemAllocator> query_allocator =
      std::make_shared<QueryAllocator>();
  std::vector<Sequence> entry_set;
  Sequence candidate;

  // Binary search, valid size of the index_block is <size - 1>
  int l = 0, r = index_block.size() - 2;  // IndexBlock shouldn't be too large,
                                          // so int for index should be enough
  while (l <= r) {
    auto mid = (l + r) / 2;
    ret = io_.ReadSSTDataBlock(file_abs_path, query_allocator, entry_set,
                               index_block[mid + 1] - index_block[mid],
                               index_block[mid]);
    if (!ret.StatusNoError())
      return ret;
    for (auto& e : entry_set) {  // Ascending order
      if (comp->Equal(e.data(), query_key.data())) {
        char* ret_entry_ptr = query_buffer->Allocate(e.size());
        std::memcpy(ret_entry_ptr, e.data(), e.size());
        ret_entry = Sequence(ret_entry_ptr, e.size());
        return ret;
      }
    }

    // Keep binary searching
    // query_key not in the DataBlock, read next block or prev block.
    if (comp->Greater(query_key.data(), entry_set.back().data())) {
      // The query_key is larger than the largest entry in the block
      l = mid + 1;
    } else {
      // The query_key is less than the smallest entry in the block
      r = mid - 1;
    }

    entry_set.clear();
  }

  return ret;
}