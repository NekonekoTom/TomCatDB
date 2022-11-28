#include "format.h"

ManifestFormat::ManifestData ManifestFormat::Decode(
    const std::string& manifest) {
  ManifestData ret;

  std::vector<std::string> lines = neko_base::Split(manifest, '\n');

  ret.path_to_manifest = lines[0];
  ret.path_to_log = lines[1];

  for (int i = 2; i < lines.size(); ++i) {
    ret.data_files.push_back(neko_base::Split(lines[i], ';'));
  }

  return ret;
}