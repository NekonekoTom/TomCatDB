// Deprecated and incompeleted

#ifndef BATCH_WRITER_H_
#define BATCH_WRITER_H_

#include "sequence.h"
#include "writer.h"

class SequentialWriter : public BaseWriter {
 public:
  SequentialWriter() = delete;
  // explicit SequentialWriter(DBFile* dbf_ptr, const int max_buffer_size = 1024, Sequence* seq = nullptr)
  //     : seq_(seq), BaseWriter(dbf_ptr, max_buffer_size) {}
  explicit SequentialWriter(DBFile* dbf_ptr, Sequence* seq = nullptr,
                       const int max_buffer_size = 1024);

  SequentialWriter(const SequentialWriter&) = delete;
  SequentialWriter& operator=(const SequentialWriter&) = delete;

  ~SequentialWriter();

  // Delete
  const int AppendToBuffer(const Sequence* seq) { return 0; }

  // Delete
  const int AppendToBuffer(const char* src, const int size) { return 0; }

 private:
  // const char* data_;
  Sequence* seq_;
};

SequentialWriter::SequentialWriter(DBFile* dbf_ptr, Sequence* seq,
                         const int max_buffer_size)
    : seq_(seq), BaseWriter(dbf_ptr, max_buffer_size) {
  const char* data = seq_->data();
  const uint64_t size = seq_->size();

  if (!AbleToBuffer(seq_->size())) {
    // Fit as much as possible into the buffer
    seq_->SkipPrefix(kMaxBufferSize);
  }
  AppendToBuffer(data, size);
}

SequentialWriter::~SequentialWriter() {}

#endif