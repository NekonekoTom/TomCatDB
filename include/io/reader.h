#ifndef READER_H_
#define READER_H_

// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <unistd.h>

#include "base.h"
#include "dbfile.h"
#include "sequence.h"
#include "status.h"

class BaseReader {
 public:
  const int kMaxBufferSize = 1024;

  BaseReader() = delete;
  explicit BaseReader(const int max_buffer_size = 1024)
      // : dbfile_(dbf_ptr),
      : kMaxBufferSize(max_buffer_size), buffer_(new char[kMaxBufferSize]) {}

  BaseReader(const BaseReader&) = delete;
  BaseReader& operator=(const BaseReader&) = delete;

  virtual ~BaseReader();

  // Params:
  // file:    a DBFile pointer points to an arbitrary file
  // ret:     std::string reference that stores read contents
  // size:    expected content length
  // offset:  offset relative to the beginning of the file,
  //          calls lseek(file->fd(), offset, SEEK_SET)
  virtual Status Read(DBFile* file, std::string& ret, const uint64_t size,
                      const ::ssize_t offset) = 0;
  
  virtual Status ReadEntire(DBFile* file, std::string& ret, const uint64_t size) = 0;

  // Access function
  char* buffer() { return buffer_; }

 protected:
  Status IsCorrectlyOpened(DBFile* file);

 private:
  char* buffer_ = nullptr;
  bool required_sync_;
};

class SequentialReader : public BaseReader {
 public:
  SequentialReader() = delete;
  SequentialReader(const SequentialReader&) = delete;
  SequentialReader& operator=(const SequentialReader&) = delete;

  explicit SequentialReader(const int max_buffer_size = 1024)
      : BaseReader(max_buffer_size) {}

  ~SequentialReader();

  // Wrapper for InternalRead()
  virtual Status Read(DBFile* file, std::string& ret, const uint64_t size,
                      const ::ssize_t offset = 0);

  virtual Status ReadEntire(DBFile* file, std::string& ret, const uint64_t size);

 private:
  virtual Status InternalRead(DBFile* file, std::string& ret,
                              const uint64_t size, const ::ssize_t offset);
};

class RandomReader : public BaseReader {
 public:
  RandomReader() = delete;
  RandomReader(const RandomReader&) = delete;
  RandomReader& operator=(const RandomReader&) = delete;

  explicit RandomReader(const int max_buffer_size = 1024)
      : BaseReader(max_buffer_size) {}

  ~RandomReader();

  virtual Status Read(DBFile* file, std::string& ret, const uint64_t size);

 private:
};

#endif