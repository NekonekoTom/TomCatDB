#include "db.h"
#include "version.h"

int main(int argc, char* argv[]) {
  std::cout << "<< MVCC demo >>\n";

  std::mutex mtx;
  Config config;
  RAIILock lock(mtx);
  TCIO mvcc_io(config.GetConfig("database_dir"));

  Manifest manifest;
  mvcc_io.ReadManifest(manifest);

  TCVersion ver1(manifest);
  TCVersion ver2(manifest);
  TCVersion ver3(manifest);

  manifest.data_files.push_back({"0"});
  manifest.data_files.push_back({"3", "4"});

  TCVersion ver4(manifest);

  TCVersionCtrl version_ctrl;

  bool flag;
  flag = version_ctrl.AppendVersion(ver1);
  flag = version_ctrl.AppendVersion(ver2);
  flag = version_ctrl.AppendVersion(ver3);
  flag = version_ctrl.AppendVersion(ver4);

  return 0;
}