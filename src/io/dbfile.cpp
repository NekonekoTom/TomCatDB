#include "dbfile.h"

DBFile::DBFile(const std::string& file_name, Mode open_mode)
    : file_name_(file_name), mode_(open_mode) {
  Open();
}

bool DBFile::Open() {
  if (IsOpened()) {
    return true;
  }
  switch (mode_) {
    // Access control code: 0644 for -rw-r--r--
    case Mode::kNewFile:
      fd_ = open(file_name_.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0644);
      break;

    case Mode::kAppend:
      fd_ = open(file_name_.c_str(), O_APPEND | O_WRONLY | O_CREAT, 0644);
      break;

    case Mode::kReadOnly:
      fd_ = open(file_name_.c_str(), O_RDONLY);
      break;

    default:
      fd_ = -1;
  }
  return IsOpened();
}

bool DBFile::Close() {
  if (!IsOpened()) {
    return false;
  }
  int status = close(fd_);
  if (status) {
    // Close unsuccessfully
    return false;
  }
  fd_ = -1;
  return true;
}