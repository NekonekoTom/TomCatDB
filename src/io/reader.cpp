#include "reader.h"

BaseReader::~BaseReader() {
  if (buffer_ != nullptr)
    delete buffer_;
}

Status BaseReader::IsCorrectlyOpened(DBFile* file) {
  // Not opened
  if (!file->IsOpened()) {
    file->set_mode(DBFile::Mode::kReadOnly);
    if (!file->Open()) {
      return Status::FileIOError("File failed to be opened.");
    }
  }
  // Wrong open mode
  if (file->mode() != DBFile::Mode::kReadOnly) {
    file->Close();
    file->set_mode(DBFile::Mode::kReadOnly);
    if (!file->Open()) {
      return Status::FileIOError("Wrong open mode for the file.");
    }
  }
  return Status::NoError();
}

SequentialReader::~SequentialReader() {}

Status SequentialReader::Read(DBFile* file, std::string& ret,
                              const uint64_t size, const ::ssize_t offset) {
  if (file == nullptr) {
    return Status::FileIOError("No specified file.");
  }

  Status ret_status = IsCorrectlyOpened(file);
  if (!ret_status.StatusNoError()) {
    ret = "";
  } else {
    ret_status = InternalRead(file, ret, size, offset);
  }

  delete file;
  return ret_status;
}

Status SequentialReader::ReadEntire(DBFile* file, std::string& ret) {
  if (file == nullptr) {
    return Status::FileIOError("No specified file.");
  }

  Status ret_status = IsCorrectlyOpened(file);
  if (!ret_status.StatusNoError()) {
    ret = "";
  } else {
    uint64_t size = lseek(file->fd(), 0, SEEK_END);
    ret_status = InternalRead(file, ret, size, 0);
  }

  delete file;
  return ret_status;
}

Status SequentialReader::InternalRead(DBFile* file, std::string& ret,
                                      const uint64_t size,
                                      const ::ssize_t offset) {
  // Move file pointer to (beginning + offset)
  // auto what = lseek(file->fd(), offset, SEEK_SET); // For test
  lseek(file->fd(), offset, SEEK_SET);

  uint64_t read_bytes = 0;
  if (size < kMaxBufferSize) {
    read_bytes = read(file->fd(), buffer(), size);
    // perror("read:"); // For test
    if (read_bytes == size) {
      // Successfully read
      ret = std::string(buffer(), read_bytes);
      return Status::NoError();
    }
  } else {
    // Allocate enough space at once
    ret = std::string(size, '\0');

    read_bytes = read(file->fd(), const_cast<char*>(ret.c_str()), size);
    if (read_bytes == size) {
      // Successfully read
      return Status::NoError();
    }
  }
  ret = "";
  return Status::FileIOError("Unable to read all " + std::to_string(size) +
                             " bytes.");
}

RandomReader::~RandomReader() {}

Status RandomReader::Read(DBFile* file, std::string& ret, const uint64_t size,
                          const ::ssize_t offset) {
  // TODO
  return Status::UndefinedError();
}

Status RandomReader::ReadEntire(DBFile* file, std::string& ret) {
  // TODO
  return Status::UndefinedError();
}

Status RandomReader::InternalRead(DBFile* file, std::string& ret,
                                  const uint64_t size, const ::ssize_t offset) {
  // TODO
  return Status::UndefinedError();
}