#include "writer.h"

BaseWriter::BaseWriter(DBFile* dbf_ptr, const int max_buffer_size)
    : kMaxBufferSize(max_buffer_size),
      // kDeviceIndentifier(Device::kUbuntu),
      buffer_(new char[kMaxBufferSize]),
      pos_(0),
      required_sync_(false),
      dbfile_(dbf_ptr) {}

const int BaseWriter::AppendToBuffer(const char* src, const int size) {
  if (!AbleToBuffer(size)) {
    std::memcpy(buffer_ + pos_, src, kMaxBufferSize - pos_);
    return kMaxBufferSize - pos_;
  }

  std::memcpy(buffer_ + pos_, src, size);
  pos_ += size;
  return size;
}

const int BaseWriter::AppendToBuffer(const Sequence& seq) {
  if (!AbleToBuffer(seq.size())) {
    std::memcpy(buffer_ + pos_, seq.data(), kMaxBufferSize - pos_);
    auto ret = kMaxBufferSize - pos_;
    pos_ = kMaxBufferSize;
    return ret;
  }

  std::memcpy(buffer_ + pos_, seq.data(), seq.size());
  pos_ += seq.size();
  return seq.size();
}

Status BaseWriter::WriteNewFile() {
  if (dbfile_ == nullptr) {
    return Status::BadArgumentError("No dbfile specified.");
  }
  if (dbfile_->mode() != DBFile::kNewFile) {
    // Try to close and reopen
    dbfile_->Close();
    dbfile_->set_mode(DBFile::Mode::kNewFile);
  }
  if (!dbfile_->IsOpened()) {
    if (!dbfile_->Open()) {
      return Status::FileIOError("Cannot open dbfile.");
    }
  }

  const int kMaxRetry = 5;
  int written_bytes = 0, retry = 0;
  while (written_bytes < pos_) {
    if (retry++ >= kMaxRetry) {
      return Status::FileIOError();
    }
    written_bytes +=
        write(dbfile_->fd(), buffer_ + written_bytes, pos_ - written_bytes);
  }

  dbfile_->Close();
  ClearBuffer();  // Reset buffer
  return Status::NoError();
}

Status BaseWriter::WriteAppendFile() {
  if (dbfile_ == nullptr) {
    return Status::BadArgumentError("No dbfile specified.");
  }
  if (dbfile_->mode() != DBFile::kAppend) {
    // Try to close and reopen
    dbfile_->Close();
    dbfile_->set_mode(DBFile::Mode::kAppend);
  }
  if (!dbfile_->IsOpened()) {
    if (!dbfile_->Open()) {
      return Status::FileIOError("Cannot open dbfile.");
    }
  }

  const int kMaxRetry = 5;
  int written_bytes = 0, retry = 0;
  while (written_bytes < pos_) {
    if (retry++ >= kMaxRetry) {
      return Status::FileIOError();
    }
    written_bytes +=
        write(dbfile_->fd(), buffer_ + written_bytes, pos_ - written_bytes);
  }

  dbfile_->Close();
  ClearBuffer();  // Reset buffer
  return Status::NoError();
}

BaseWriter::~BaseWriter() {
  if (buffer_ != nullptr) {
    delete buffer_;
  }
  if (dbfile_ != nullptr) {
    delete dbfile_;
  }
}

BatchWriter::BatchWriter(DBFile* dbf_ptr, const int max_buffer_size)
    : BaseWriter(dbf_ptr, max_buffer_size) {}

Status BatchWriter::WriteBatch(const std::vector<Sequence>& entries) {
  return WriteBatch(entries, 0, entries.size());
}

Status BatchWriter::WriteBatch(const std::vector<Sequence>& entries,
                               std::vector<Sequence>::size_type begin,
                               std::vector<Sequence>::size_type end) {
  int buffered_bytes = 0;
  Status ret;
  Sequence e;  // Iteration variable
  for (auto i = begin; i < end; ++i) {
    e = entries[i];
    buffered_bytes = AppendToBuffer(e);
    while (buffered_bytes != entries[i].size()) {
      e.SkipPrefix(buffered_bytes);
      if (!(ret = WriteAppendFile()).StatusNoError()) {
        return ret;
      }
      buffered_bytes = AppendToBuffer(entries[i]);
    }
  }

  if (pos() != 0) {
    ret = WriteAppendFile();
  }

  return ret;
}

Status BatchWriter::WriteBatch(const std::vector<const char*>& entries) {
  std::vector<Sequence> seq_entries;
  for (const char* e : entries) {
    seq_entries.push_back(InternalEntry::EntryData(e));
  }
  return WriteBatch(seq_entries);
}

Status BatchWriter::WriteFragment(const char* fragment, int fragment_size) {
  Status ret;
  int buffered_bytes = 0;
  ClearBuffer();
  while (fragment_size > 0) {
    buffered_bytes = AppendToBuffer(fragment, fragment_size);
    fragment_size -= buffered_bytes;
    fragment += buffered_bytes;
    ret = WriteAppendFile();
    if (!ret.StatusNoError())
      break;
  }
  return ret;
}