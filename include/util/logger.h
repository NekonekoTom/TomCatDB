#ifndef LOGGER_H_
#define LOGGER_H_

#include "base.h"
#include "status.h"
#include "writer.h"

class TCLogger {
 public:
  TCLogger() = delete;
  TCLogger(const TCLogger&) = delete;
  TCLogger& operator=(const TCLogger&) = delete;

  TCLogger(const std::string& log_path);

  virtual ~TCLogger() = default;

  virtual Status Debug(const std::string& msg);

  virtual Status Info(const std::string& msg);

  virtual Status Warn(const std::string& msg);

  virtual Status Error(const std::string& msg);

  virtual Status Fatal(const std::string& msg);

  // virtual Status Log(const std::string& msg);

 private:
  enum LogLevel { kDebug, kInfo, kWarn, kError, kFatal };

  virtual std::string FormatMsg(const std::string& msg, const LogLevel& level);

  std::shared_ptr<SequentialWriter> log_writer_;

  std::string log_path_;

  bool async_log_ = false;

  bool log_time_ = true;
};

#endif