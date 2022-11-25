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
  const int kMaxBufferSize;

  BaseReader() = delete;
  explicit BaseReader(const int max_buffer_size = 1024)
      // : dbfile_(dbf_ptr),
      : kMaxBufferSize(max_buffer_size), buffer_(new char[kMaxBufferSize]) {}

  BaseReader(const BaseReader&) = delete;
  BaseReader& operator=(const BaseReader&) = delete;

  virtual ~BaseReader();

  // Read contents started from the offset of the file beginning.
  // Params:
  // file:    a DBFile pointer points to an arbitrary file
  // ret:     std::string reference that stores read contents
  // size:    expected content length
  // offset:  offset relative to the beginning of the file,
  //          calls lseek(file->fd(), offset, SEEK_SET)
  virtual Status Read(DBFile* file, std::string& ret, const uint64_t size,
                      const ::ssize_t offset) = 0;
  

  // Read all contents from the file and store them in std::string
  virtual Status ReadEntire(DBFile* file, std::string& ret) = 0;

 protected:
  // Calling this function will possibly try to open or re-open the DBFile.
  // Returns Status::FileIOError when the attempt failed.
  Status IsCorrectlyOpened(DBFile* file);

  // Access function
  char* buffer() { return buffer_; }

 private:
  char* buffer_ = nullptr;
  bool required_sync_ = false;
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

  virtual Status ReadEntire(DBFile* file, std::string& ret);

 private:
  // Before calling this function, user should ensure the DBFile ptr is valid.
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

  // Wrapper for InternalRead()
  virtual Status Read(DBFile* file, std::string& ret, const uint64_t size,
                      const ::ssize_t offset = 0);

  virtual Status ReadEntire(DBFile* file, std::string& ret);

 private:
  // Before calling this function, user should ensure the DBFile ptr is valid.
  virtual Status InternalRead(DBFile* file, std::string& ret,
                              const uint64_t size, const ::ssize_t offset);
};

#endif