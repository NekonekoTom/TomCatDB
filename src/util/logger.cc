#include "logger.h"

TCLogger::TCLogger(const std::string& log_path) {
  log_writer_ = std::make_shared<SequentialWriter>(
      new DBFile(log_path, DBFile::Mode::kAppend));
}

Status TCLogger::Debug(const std::string& msg) {
  Status ret;

  auto log_msg = FormatMsg(msg, kDebug);

  if (!async_log_) {
    ret = log_writer_->WriteFragment(log_msg);
  }

  return ret;
}

Status TCLogger::Info(const std::string& msg) {
  // TODO
  return Debug(msg);
}

Status TCLogger::Warn(const std::string& msg) {
  // TODO
  return Debug(msg);
}

Status TCLogger::Error(const std::string& msg) {
  // TODO
  return Debug(msg);
}

Status TCLogger::Fatal(const std::string& msg) {
  // TODO
  return Debug(msg);
}

std::string TCLogger::FormatMsg(const std::string& msg, const LogLevel& level) {
  std::string log_msg;
  switch (level) {
    case kDebug:
      log_msg = "[Debug] - " + msg + "\n";
      break;
    case kInfo:
      log_msg = "[Info] - " + msg + "\n";
      break;
    case kWarn:
      log_msg = "[Warn] - " + msg + "\n";
      break;
    case kError:
      log_msg = "[Error] - " + msg + "\n";
      break;
    case kFatal:
      log_msg = "[Fatal] - " + msg + "\n";
      break;
    default:
      log_msg = "[Debug] - " + msg + "\n";
      break;
  }
  return log_msg;
}
