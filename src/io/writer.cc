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
    auto ret = kMaxBufferSize - pos_;
    pos_ = kMaxBufferSize;
    return ret;
  }

  std::memcpy(buffer_ + pos_, src, size);
  pos_ += size;
  return size;
}

const int BaseWriter::AppendToBuffer(const Sequence& seq) {
  return AppendToBuffer(seq.data(), seq.size());
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

// Reimplement BaseWriter::WriteAppendFile()
Status BaseWriter::WriteAppendFile() {
  if (dbfile_ == nullptr) {
    return Status::BadArgumentError("No dbfile specified.");
  }

  int max_retry = 5;
  int fd = -1;
  while (max_retry-- >= 0 && fd < 0) {
    fd =
        open(dbfile_->file_name().c_str(), O_APPEND | O_WRONLY | O_CREAT, 0644);
  }

  if (fd < 0) {
    return Status::FileIOError("Cannot open dbfile.");
  }

  // Write until succeed
  int written_bytes = 0;
  while (written_bytes < pos_) {
    int write_stat = write(fd, buffer_ + written_bytes, pos_ - written_bytes);
    if (write_stat > 0)
      written_bytes += write_stat;
  }

  while (close(fd) < 0);
  ClearBuffer();  // Reset buffer
  return Status::NoError();
}

BaseWriter::~BaseWriter() {
  if (buffer_ != nullptr) {
    delete[] buffer_;
  }
  if (dbfile_ != nullptr) {
    delete dbfile_;
  }
}

SequentialWriter::SequentialWriter(DBFile* dbf_ptr, const int max_buffer_size)
    : BaseWriter(dbf_ptr, max_buffer_size) {}

Status SequentialWriter::WriteBatch(const std::vector<Sequence>& entries) {
  return WriteBatch(entries, 0, entries.size());
}

Status SequentialWriter::WriteBatch(const std::vector<Sequence>& entries,
                                    std::vector<Sequence>::size_type begin,
                                    std::vector<Sequence>::size_type end) {
  int buffered_bytes = 0;
  Status ret;
  Sequence e;  // Iteration variable
  for (auto i = begin; i < end; ++i) {
    e = entries[i];
    buffered_bytes = AppendToBuffer(e);
    while (buffered_bytes != e.size()) {
      e.SkipPrefix(buffered_bytes);
      if (!(ret = WriteAppendFile()).StatusNoError()) {
        return ret;
      }
      buffered_bytes = AppendToBuffer(e);
    }
  }

  if (pos() != 0) {
    ret = WriteAppendFile();
  }

  return ret;
}

Status SequentialWriter::WriteBatch(const std::vector<const char*>& entries) {
  std::vector<Sequence> seq_entries;
  for (const char* e : entries) {
    seq_entries.push_back(InternalEntry::EntryData(e));
  }
  return WriteBatch(seq_entries);
}

Status SequentialWriter::WriteFragment(const char* fragment,
                                       int fragment_size) {
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