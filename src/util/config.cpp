#include "config.h"

Config::Config() {
  AddOrUpdateConfig("database_dir", kDefaultDatabaseDir);
}

Config::~Config() {}

void Config::AddOrUpdateConfig(const std::string& name,
                               const std::string& option) {
  config_[name] = option;
}

const std::string Config::GetConfig(const std::string& name) const {
  if (config_.find(name) != config_.end()) {
    // return config_[name]; // non-const
    return config_.at(name);
  }
  return std::string("");
}

// std::string DBFile::ToAbsolutePath(const std::string& file_name) {
//   if (file_name[0] == '.') {
//     std::string curdir = get_current_dir_name();
//     // Skip this module's relative path: ./src/io
//     curdir = std::string(curdir.begin(), curdir.end() - 7);
//     return curdir + "/" + file_name.substr(2, file_name.size() - 2);
//   }
//   return file_name;
// }