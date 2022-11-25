#ifndef FORMAT_H_
#define FORMAT_H_

class FileFormat {
 public:
  FileFormat() = delete;
  FileFormat(const FileFormat&) = delete;
  FileFormat& operator=(const FileFormat&) = delete;
  ~FileFormat() = default;

 private:
};

class ManifestFormat : public FileFormat {
 public:
  ManifestFormat() = delete;
  ManifestFormat(const ManifestFormat&) = delete;
  ManifestFormat& operator=(const ManifestFormat&) = delete;
  ~ManifestFormat() = default;

 private:
};

// TODO
class LogFormat : public FileFormat {
 public:
  LogFormat() = delete;
  LogFormat(const LogFormat&) = delete;
  LogFormat& operator=(const LogFormat&) = delete;
  ~LogFormat() = default;

 private:
};

#endif