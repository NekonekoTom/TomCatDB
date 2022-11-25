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
    return kMaxBufferSize - pos_;
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
    written_bytes += write(dbfile_->fd(), buffer_ + written_bytes, pos_ - written_bytes);
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
