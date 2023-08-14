#include "version.h"

std::atomic<uint64_t> TCVersion::inc_id_;  // Call default constructor

bool TCVersionCtrl::AppendVersion(const TCVersion& version) {
  auto manifest_str = version.ManifestStr();

  std::lock_guard<std::mutex> lock(v_mutex_);

  if (!ExistVersion(version)) {
    // Insert the version into the version_list_
    version_list_.push_back(version);

    // Insert serialized manifest of the version as the key,
    // the last element in the version_list_ as the value
    version_map_[manifest_str] = --version_list_.end();

    return true;
  } else {
    version_map_[manifest_str]->Ref();
    return false;
  }
}

Status TCVersionCtrl::UnrefVersion(const TCVersion& version) {
  auto manifest_str = version.ManifestStr();

  {
    std::lock_guard<std::mutex> lock(v_mutex_);

    if (!ExistVersion(manifest_str)) {
      return Status::BadArgumentError("The version does not exist");
    }

    version_map_[manifest_str]->UnRef();
    if (version_map_[manifest_str]->Evictable()) {
      // Evict from the version_list_
      version_list_.erase(version_map_[manifest_str]);

      // Evict from the version_map_
      version_map_.erase(manifest_str);
    }
  }

  return Status::NoError();
}

bool TCVersionCtrl::ExistVersion(const TCVersion& version) const {
  // No check
  std::string manifest_str = version.ManifestStr();

  return version_map_.find(manifest_str) != version_map_.end();
}

bool TCVersionCtrl::ExistVersion(const Manifest& manifest) const {
  // No check
  std::string manifest_str = ManifestFormat::Encode(manifest);

  return version_map_.find(manifest_str) != version_map_.end();
}