#include "reader.h"

BaseReader::~BaseReader() {
  if (buffer_ != nullptr)
    delete buffer_;
  // if (dbfile_ != nullptr)
  //   delete dbfile_;
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

  Status ret_status = InternalRead(file, ret, size, offset);

  delete file;
  return ret_status;
}

Status SequentialReader::ReadEntire(DBFile* file, std::string& ret,
                                    const uint64_t size) {
  if (file == nullptr) {
    return Status::FileIOError("No specified file.");
  }

  Status ret_status = InternalRead(file, ret, size, 0);

  delete file;
  return ret_status;
}

Status SequentialReader::InternalRead(DBFile* file, std::string& ret,
                                      const uint64_t size,
                                      const ::ssize_t offset) {
  // Calling this function will possibly try to open or re-open the DBFile.
  // Returns Status::FileIOError when the attempt failed.
  IsCorrectlyOpened(file);

  // Move file pointer to (current position + offset)
  // auto what = lseek(file->fd(), offset, SEEK_CUR); // For test
  lseek(file->fd(), offset, SEEK_SET);

  uint64_t read_bytes = 0;
  if (size < kMaxBufferSize) {
    read_bytes = read(file->fd(), buffer(), size);
    // perror("read:"); // For test
    if (read_bytes == size) {
      // Successful read
      ret = std::string(buffer(), read_bytes);
      return Status::NoError();
    } else {
      ret = "";
      return Status::FileIOError("Unable to read all " + std::to_string(size) +
                                 " bytes.");
    }
  } else {
    while (size - read_bytes >= kMaxBufferSize) {
      uint64_t rb = read_bytes;
      read_bytes += read(file->fd(), buffer(), kMaxBufferSize);
      if (read_bytes == rb + kMaxBufferSize) {
        // Successful read
        ret += std::string(buffer(), kMaxBufferSize);
        // Do not need to clear the buffer,
        // because new data will overwrite the buffer space.
      } else {
        return Status::FileIOError("Unable to read all " +
                                   std::to_string(size) + " bytes.");
      }
    }
    // Last iter
    read_bytes += read(file->fd(), buffer(), size - read_bytes);
    if (read_bytes == size) {
      // Successful read
      ret += std::string(buffer(), size % kMaxBufferSize);
      return Status::NoError();
    } else {
      ret = "";
      return Status::FileIOError("Unable to read all " + std::to_string(size) +
                                 " bytes.");
    }
  }
}

RandomReader::~RandomReader() {}

Status RandomReader::Read(DBFile* file, std::string& ret, const uint64_t size) {
  // TODO

  return Status::FileIOError();
}