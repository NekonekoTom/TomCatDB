#ifndef WRITER_H_
#define WRITER_H_

// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <unistd.h>

#include "base.h"
#include "dbfile.h"
#include "sequence.h"
#include "status.h"

class BaseWriter {
 public:
  const int kMaxBufferSize = 1024;

  BaseWriter() = default;
  explicit BaseWriter(DBFile* dbf_ptr, const int max_buffer_size = 1024);

  BaseWriter(const BaseWriter&) = delete;
  BaseWriter& operator=(const BaseWriter&) = delete;

  virtual ~BaseWriter();

  // Return buffered bytes count
  virtual const int AppendToBuffer(const Sequence& seq);

  // Return buffered bytes count
  virtual const int AppendToBuffer(const char* src, const int size);

  // Flush contents in buffer_ to dbfile_ by creating
  // a new file or truncating an existing file
  virtual Status WriteNewFile();

  // Flush contents in buffer_ to dbfile_ by appending
  virtual Status WriteAppendFile();

  void ClearBuffer() { pos_ = 0; }
  bool AbleToBuffer(const int size) const {
    return pos_ + size <= kMaxBufferSize;
  }

  const int pos() const { return pos_; }

 private:
  char* buffer_;
  int pos_;  // Points to the end of char sequence in buffer_
  bool required_sync_;
  DBFile* dbfile_;
};

#endif