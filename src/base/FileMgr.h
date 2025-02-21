#pragma once
#include "FileLog.h"
#include "NonCopyable.h"
#include "Singleton.h" 
#include <memory>
#include <mutex>
#include <unordered_map>
namespace tmms
{
  namespace base
  {
    class FileMgr : public NonCopyable
    {
    public:
      FileMgr() = default;
      ~FileMgr() = default;
      
      void OnCheck();
      FileLogPtr GetFileLog(const std::string &file_name);
      void RemoveFileLog(const FileLogPtr &log);
      void RotateDays(const FileLogPtr &log);
      void RotateHours(const FileLogPtr &log);
      void RotateMinutes(const FileLogPtr &log);
    private:
      std::unordered_map<std::string, FileLogPtr> logs_;
      std::mutex lock_;
      int last_day_{-1};
      int last_hour_{-1};
      int last_minute_{-1};
      int last_year_{-1};
      int last_month_{-1};
    };
  }
}

#define sFileMgr tmms::base::Singleton<tmms::base::FileMgr>::Instance()
