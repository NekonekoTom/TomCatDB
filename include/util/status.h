#ifndef STATUS_H_
#define STATUS_H_

#include <string>

class Status {
 public:
  Status() = default;
  ~Status() = default;

  // Return true if the Status object refers to NoError
  bool StatusNoError() { return err_type_ == kOk; }

  static Status NoError() { return Status(); }

  static Status UndefinedError(const std::string& err_msg = std::string()) {
    return Status(Code::kUndefinedError, "<ERROR>" + err_msg);
  }

  static Status FileIOError(const std::string& err_msg = std::string()) {
    return Status(Code::kFileIOError, "<ERROR>" + err_msg);
  }

  static Status BadArgumentError(const std::string& err_msg = std::string()) {
    return Status(Code::kBadArgumentError, "<ERROR>" + err_msg);
  }

 private:
  enum Code {
    kOk = 0,
    kUndefinedError = 1,
    kFileIOError = 2,
    kBadArgumentError = 3
  };

  Status(Code err_type, const std::string& err_msg = std::string())
      : err_type_(err_type), err_msg_(err_msg) {}

  Code err_type_;
  std::string err_msg_;
};

#endif