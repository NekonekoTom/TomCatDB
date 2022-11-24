#include "io_manager.h"

IOManager::IOManager(const std::string& files_dir) : kDatabaseDir(files_dir) {
  // If the kDatabaseDir does not exist or does not have rwx permissions
  if (access(kDatabaseDir.c_str(), F_OK | W_OK | X_OK) != 0) {
    // Assert fails when the directory did not have the desired permissions
    // or mkdir() operation failed
    assert(mkdir(kDatabaseDir.c_str(), 0744) == 0); // Must have rwx permission
  }

  BuildFile(&manifest_, kManifestFilename);

  BuildFile(&log_file_, kLogFilename);
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
  // TODO
  // Read from manifest

  // If level0 file num exceed kDefaultLevel0FileNum, push level0 to level1

  // Write level0 file
}

void IOManager::BuildFile(DBFile** file, const std::string& filename) {
  *file = new DBFile(kDatabaseDir + "/" + filename, DBFile::Mode::kNewFile);
  assert((*file)->IsOpened());

  BaseWriter* writer = new BaseWriter(*file);
  writer->AppendToBuffer(Sequence((*file)->file_name()));
  writer->WriteNewFile();

  writers_.push_back(writer);
}