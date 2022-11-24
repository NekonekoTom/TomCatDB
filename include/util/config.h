#ifndef CONFIG_H_
#define CONFIG_H_

#include <unordered_map>

class Config {
 public:
  Config();
  ~Config();

  void AddOrUpdateConfig(const std::string& name, const std::string& option);

  const std::string GetConfig(const std::string& name) const;

 private:
  const std::string kDefaultDatabaseDir = "./db";

  std::unordered_map<std::string, std::string> config_;
};

#endif