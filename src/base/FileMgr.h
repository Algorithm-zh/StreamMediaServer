#pragma once

#include "FileLog.h"
#include "Singleton.h"
#include <memory>
#include <unordered_map>
#include <mutex>

namespace tmms
{
    namespace base
    {
        

        class FileMgr:public NonCopyable
        {
        public:
            FileMgr() = default;
            ~FileMgr() = default;

            void OnCheck();
            FileLogPtr GetFileLog(const std::string &file_name);
            void RemoveFileLog(const FileLogPtr &log);
            void RotateDays(const FileLogPtr &file);
            void RotateHours(const FileLogPtr &file);
            void RotateMinutes(const FileLogPtr &file);
        private:
            std::unordered_map<std::string,FileLogPtr> logs_;
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