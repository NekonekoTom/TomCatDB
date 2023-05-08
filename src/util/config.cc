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