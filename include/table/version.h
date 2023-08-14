#ifndef VERSION_H_
#define VERSION_H_

#include "base.h"
#include "dual_list.h"
#include "format.h"

class TCVersion {
 public:
  explicit TCVersion(const Manifest& manifest) : manifest_(manifest) {
    ref_.store(1);
    version_id_ = inc_id_++;
  }

  explicit TCVersion(const TCVersion& version) {
    this->ref_.store(version.ref_);
    manifest_ = version.manifest_;
    version_id_ = version.version_id_;
  }

  TCVersion() = delete;
  TCVersion& operator=(const TCVersion&) = delete;

  ~TCVersion() = default;

  std::string ManifestStr() const { return ManifestFormat::Encode(manifest_); }

  void Ref() { ++ref_; }

  void UnRef() { --ref_; }

  bool Evictable() const { return ref_.load() <= 0; }

  void set_manifest(const Manifest& manifest) { manifest_ = manifest; }

  Manifest manifest() const { return manifest_; }

 private:
  static std::atomic<uint64_t> inc_id_;

  std::atomic<uint32_t> ref_;

  Manifest manifest_;

  // Unique version id
  uint64_t version_id_;
};

class TCVersionCtrl {
 public:
  TCVersionCtrl() {}

  TCVersionCtrl(const TCVersionCtrl&) = delete;
  TCVersionCtrl& operator=(const TCVersionCtrl&) = delete;

  ~TCVersionCtrl() {}

  // Try appending a version to the version_list_.
  // This function returns either true if the version does not exist in the
  // version_list_, or false if the version already exists in the list.
  bool AppendVersion(const TCVersion& version);

  // Ref a version in the version_list_. Usually, a version will be referenced
  // by the user by calling LatestVersion(), so parameter in RefVersion should
  // be std::list<TCVersion>::const_iterator.
  // Directly calling this function is concurrently UNSAFE.
  Status RefVersion(std::list<TCVersion>::iterator& version_it) {
    version_it->Ref();
    return Status::NoError();  // TODO
  }

  // Unref a version from the version_list_. If the ref_ of the version
  // decreased to 0, evict it from the version_list_.
  Status UnrefVersion(const TCVersion& version);

  // Return the latest version's const_iterator.
  std::list<TCVersion>::const_iterator LatestVersion() {
    std::lock_guard<std::mutex> lock(v_mutex_);
    auto version = --version_list_.end();
    RefVersion(version);
    return version;
  }

  bool ExistVersion(const TCVersion& version) const;

  bool ExistVersion(const Manifest& manifest) const;

  bool ExistVersion(const std::string& manifest) const {
    return version_map_.find(manifest) != version_map_.end();
  }

 private:
  std::mutex v_mutex_;

  std::unordered_map<std::string, std::list<TCVersion>::iterator> version_map_;

  // neko_base::DualList<TCVersion> version_list_;
  std::list<TCVersion> version_list_;
};

#endif