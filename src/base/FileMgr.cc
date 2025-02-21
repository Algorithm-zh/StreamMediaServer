#include "FileMgr.h"
#include "TTime.h"
#include "StringUtils.h"
#include <sstream>
using namespace tmms::base;
// 
namespace 
{
  static tmms::base::FileLogPtr file_log_nullptr;  
}
void FileMgr::OnCheck()  {

  bool day_change{false};
  bool hour_change{false};
  bool minute_change{false};
  int year = 0, month = 0, day = -1, hour = -1, minute = 0, second = 0;
  TTime::Now(year, month, day, hour, minute, second);
  if(last_day_ == -1)
  {
    last_day_ = day;
    last_hour_ = hour;
    last_minute_ = minute;
  }
  if(last_day_ != day)
  {
    day_change = true;
  }
  if(last_hour_ != hour)
  {
    hour_change = true;
  }
  if(last_minute_ != minute)
  {
    minute_change = true;
  }
  if(!day_change && !hour_change && !minute_change)
    return;


  std::lock_guard<std::mutex> lk(lock_);
  for(auto &l : logs_)
  {
    if(minute_change && l.second->GetRotateType() == kRotateMinute)
    {
      RotateMinutes(l.second);
    }
    //按日期切分日志
    if(day_change && l.second->GetRotateType() == kRotateDay)
    {
      RotateDays(l.second);
    }
    //按小时切分
    if(hour_change && l.second->GetRotateType() == kRotateHour)
    {
      RotateHours(l.second);
    }
  }

  last_day_ = day;
  last_hour_ = hour;
  last_year_ = year;
  last_minute_ = minute;
  last_month_ = month;
}
 
FileLogPtr FileMgr::GetFileLog(const std::string &file_name)  {
  std::lock_guard<std::mutex> lk(lock_);
  auto iter = logs_.find(file_name);
  if(iter != logs_.end())
  {
    return iter->second;
  }
  FileLogPtr log = std::make_shared<FileLog>();
  if(!log->Open(file_name))
  {
    return file_log_nullptr;
  }
  logs_.emplace(file_name, log);
  return log;
}
 
void FileMgr::RemoveFileLog(const FileLogPtr &log)  {

  std::lock_guard<std::mutex> lk(lock_);
  logs_.erase(log->FilePath());
}
 
void FileMgr::RotateDays(const FileLogPtr &log)  {
  
  if(log->FileSize() > 0)
  {
    char buf[128] = {0};
    sprintf(buf, "_%04d-%02d-%02d", last_year_, last_month_, last_day_);
    std::string file_path = log->FilePath();
    std::string path = StringUtils::FilePath(file_path);
    std::string file_name = StringUtils::FileName(file_path);
    std::string file_ext = StringUtils::Extension(file_path);

    //可以将不同类型的变量合并为一个字符串
    std::ostringstream ss;
    ss << path << file_name << buf << file_ext;
    log->Rotate(ss.str());
  }
}
 
void FileMgr::RotateHours(const FileLogPtr &log)  {

 if(log->FileSize() > 0)
  {
    char buf[128] = {0};
    sprintf(buf, "_%04d-%02d-%02dT%02d", last_year_, last_month_, last_day_, last_hour_);
    std::string file_path = log->FilePath();
    std::string path = StringUtils::FilePath(file_path);
    std::string file_name = StringUtils::FileName(file_path);
    std::string file_ext = StringUtils::Extension(file_path);

    //可以将不同类型的变量合并为一个字符串
    std::ostringstream ss;
    ss << path << file_name << buf << file_ext;
    log->Rotate(ss.str());
  }
} 
void FileMgr::RotateMinutes(const FileLogPtr &log)  {

 if(log->FileSize() > 0)
  {
    char buf[128] = {0};
    sprintf(buf, "_%04d-%02d-%02dT%02d:%02d", last_year_, last_month_, last_day_, last_hour_, last_minute_);
    std::string file_path = log->FilePath();
    std::string path = StringUtils::FilePath(file_path);
    std::string file_name = StringUtils::FileName(file_path);
    std::string file_ext = StringUtils::Extension(file_path);

    //可以将不同类型的变量合并为一个字符串
    std::ostringstream ss;
    ss << path << file_name << buf << file_ext;
    log->Rotate(ss.str());
  }
}
