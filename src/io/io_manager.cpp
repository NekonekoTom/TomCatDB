#include "io_manager.h"

IOManager::IOManager(const std::string& files_dir) : kDatabaseDir(files_dir) {
  // If the kDatabaseDir does not exist or does not have rwx permissions
  if (access(kDatabaseDir.c_str(), F_OK | W_OK | X_OK) != 0) {
    // Assert fails when the directory did not have the desired permissions
    // or mkdir() operation failed
    assert(mkdir(kDatabaseDir.c_str(), 0744) == 0);  // Must have rwx permission
  }

  // Push manifest writer into writers_
  BuildMetadataFile(&manifest_, kManifestFilename);

  // For test
  writers_[0]->AppendToBuffer(Sequence("\n00000000.tdb"));
  writers_[0]->WriteAppendFile();

  // Push log writer into writers_
  BuildMetadataFile(&log_file_, kLogFilename);

  for (int i = 0; i < kDefaultReaderNum; ++i)
    readers_.push_back(new SequentialReader(kDefaultReaderBufferSize));
}

IOManager::~IOManager() {
  // DO NOT delete manifest_ or other DBFile ptr here.
  // The mission will be done by deconstructors of the writers.
  // delete manifest_;

  for (auto w : writers_) {
    delete w;
  }
}

Status IOManager::WriteLevel0File() {
  // Read from manifest
  if (readers_.empty()) {
    return Status::FileIOError("No available Reader.");
  }

  SequentialReader* reader = readers_.back();
  readers_.pop_back();

  std::string manifest_content;
  Status ret =
      reader->ReadEntire(new DBFile(kDatabaseDir + "/" + kManifestFilename,
                                    DBFile::Mode::kReadOnly),
                         manifest_content);
  if (!ret.StatusNoError()) {
    return ret;
  }

  // If level0 file num exceed kDefaultLevel0FileNum, push level0 to level1

  // Write level0 file

  return Status::UndefinedError();
}

void IOManager::BuildMetadataFile(DBFile** file, const std::string& filename) {
  *file = new DBFile(kDatabaseDir + "/" + filename, DBFile::Mode::kNewFile);
  assert((*file)->IsOpened());

  BaseWriter* writer = new BaseWriter(*file);
  writer->AppendToBuffer(Sequence((*file)->file_name()));
  writer->WriteNewFile();

  writers_.push_back(writer);
}