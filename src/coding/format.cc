#include "format.h"

ManifestFormat::ManifestData ManifestFormat::Decode(
    const std::string& manifest_str) {
  ManifestData ret;

  std::vector<std::string> lines = neko_base::Split(manifest_str, '\n');

  ret.path_to_manifest = lines[0];
  ret.path_to_log = lines[1];

  for (int i = 2; i < lines.size(); ++i) {
    ret.data_files.push_back(neko_base::Split(lines[i], ';'));
  }

  return ret;
}

std::string ManifestFormat::Encode(const ManifestData& manifest) {
  std::string ret;
  char seperator = '\n';
  ret = manifest.path_to_manifest + seperator + manifest.path_to_log + seperator;
  for (auto& level : manifest.data_files) {
    std::string line;
    for (auto& f : level) {
      line += f + ';';
    }
    if (line.empty())
      line.push_back('\n');
    else
      line.back() = '\n';
    ret += line;
  }
  return ret;
}