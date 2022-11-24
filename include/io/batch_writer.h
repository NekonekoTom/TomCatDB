// Deprecated and incompeleted

#ifndef BATCH_WRITER_H_
#define BATCH_WRITER_H_

#include "sequence.h"
#include "writer.h"

class BatchWriter : public BaseWriter {
 public:
  BatchWriter() = delete;
  // explicit BatchWriter(DBFile* dbf_ptr, const int max_buffer_size = 1024, Sequence* seq = nullptr)
  //     : seq_(seq), BaseWriter(dbf_ptr, max_buffer_size) {}
  explicit BatchWriter(DBFile* dbf_ptr, Sequence* seq = nullptr,
                       const int max_buffer_size = 1024);

  BatchWriter(const BatchWriter&) = delete;
  BatchWriter& operator=(const BatchWriter&) = delete;

  ~BatchWriter();

  // Delete
  const int AppendToBuffer(const Sequence* seq) { return 0; }

  // Delete
  const int AppendToBuffer(const char* src, const int size) { return 0; }

 private:
  // const char* data_;
  Sequence* seq_;
};

BatchWriter::BatchWriter(DBFile* dbf_ptr, Sequence* seq,
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

BatchWriter::~BatchWriter() {}

#endif