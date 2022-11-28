#ifndef WRITER_H_
#define WRITER_H_

// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <unistd.h>

#include "base.h"
#include "dbfile.h"
#include "internal_entry.h"
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
  char* buffer_ = nullptr;
  int pos_ = 0;  // Points to the end of char sequence in buffer_
  bool required_sync_ = false;
  DBFile* dbfile_ = nullptr;
};

class BatchWriter : private BaseWriter {
 public:
  BatchWriter() = default;
  explicit BatchWriter(DBFile* dbf_ptr, const int max_buffer_size = 1024);

  BatchWriter(const BatchWriter&) = delete;
  BatchWriter& operator=(const BatchWriter&) = delete;

  ~BatchWriter() = default;

  // Write a Sequence batch to DBFile, call the second WriteBatch below
  Status WriteBatch(const std::vector<Sequence>& entries);

  // Write a Sequence batch to DBFile,
  // the batch ranges from entries[begin] to entries[end - 1]
  Status WriteBatch(const std::vector<Sequence>& entries,
                    std::vector<Sequence>::size_type begin,
                    std::vector<Sequence>::size_type end);

  // Write a const char* batch to DBFile, call WriteBatch()(Sequence version)
  Status WriteBatch(const std::vector<const char*>& entries);

  // Convenient function for the purpose of reusing BatchWriter object
  // to quickly write a short char sequence.
  Status WriteFragment(const char* fragment, int fragment_size);

 private:
  // TODO: Any data members? Use static functions instead?
};

#endif