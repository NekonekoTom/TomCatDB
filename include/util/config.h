#ifndef CONFIG_H_
#define CONFIG_H_

#include <unistd.h>  // For get_current_dir_name()
#include <unordered_map>

class Config {
 public:
  Config();
  ~Config();

  void AddOrUpdateConfig(const std::string& name, const std::string& option);

  const std::string GetConfig(const std::string& name) const;

 private:
  // For test, maybe set to /usr ?
  const std::string kDefaultDatabaseDir =
      "/home/tom_cat/workdir/private/CS/C++/Primer/TomCatDB/db";

  std::unordered_map<std::string, std::string> config_;
};

#endif