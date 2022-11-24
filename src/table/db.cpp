#include "db.h"

TCDB::TCDB(const Config& config)
    : global_lock_(mutex_),
      volatile_table_(new TCTable(global_lock_)),
      io_manager_(config.GetConfig("database_dir")) {}

TCDB::~TCDB() {}

const Sequence* TCDB::Get(const Sequence& key) {
  // TODO
  return volatile_table_->Get(key);
}

Status TCDB::Insert(const Sequence& key, const Sequence& value) {
  // TODO
  return volatile_table_->Insert(key, value);
}

Status TCDB::Delete(const Sequence& key) {
  // TODO
  return volatile_table_->Delete(key);
}

bool TCDB::ContainsKey(const Sequence& key) {
  // TODO
  return volatile_table_->ContainsKey(key);
}

Status TCDB::TransferTable(const TCTable** immutable) {
  global_lock_.Lock();
  *immutable = volatile_table_;
  volatile_table_ = new TCTable(global_lock_);
  global_lock_.Unlock();

  return *immutable != nullptr ? Status::NoError() : Status::UndefinedError();
}

Status TCDB::WriteLevel0() {
  const TCTable* immutable = nullptr;

  if (!TransferTable(&immutable).StatusNoError()) {
    // The volatile_table_ object failed to be transfered
    return Status::UndefinedError();
  }

  io_manager_.WriteLevel0File();

  return Status::NoError();
}