#pragma once
#include <string>
#include <memory>

namespace tmms
{
  namespace base
  {
    enum RotateType
    {
      kRotateNone,
      kRotateMinute,
      kRotateHour,
      kRotateDay,
    };
    class FileLog
    {
    public:
      FileLog() = default;
      ~FileLog() = default;
      
      bool Open(const std::string &filePath);
      size_t WriteLog(const std::string &msg);
      void Rotate(const std::string &file);// 日志切分
      void setRotate(RotateType type);
      RotateType GetRotateType() const;
      int64_t FileSize() const;
      std::string FilePath() const;
    private:
      int fd_ = -1;
      std::string file_path_;
      RotateType rotate_type_{kRotateNone};
    };
    using FileLogPtr = std::shared_ptr<FileLog>;
  }
}
