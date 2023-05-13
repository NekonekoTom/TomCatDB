#ifndef FILE_H_
#define FILE_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

// Do not recommend directly delete(free) the DBFile pointers in Writers.
// Users should deliver this mission to the Writer objects.
class DBFile final {
 public:
  enum Mode { kNewFile = 0, kAppend = 1, kReadOnly = 2 };

  DBFile() = default;
  explicit DBFile(const std::string& file_name) : file_name_(file_name) {
    Open();
  }  // Open as read-only
  DBFile(const std::string& file_name, Mode open_mode);

  DBFile(const DBFile&) = delete;
  DBFile& operator=(const DBFile&) = delete;

  ~DBFile() { Close(); }

  bool Open();
  bool Close();

  bool IsOpened() const { return fd_ >= 0; }
  void set_mode(Mode new_mode) { mode_ = new_mode; }
  int fd() const { return fd_; }
  Mode mode() const { return mode_; }
  const std::string& file_name() const { return file_name_; }

 private:
  int fd_ = -1;
  Mode mode_ = Mode::kReadOnly;
  std::string file_name_;
};

#endif