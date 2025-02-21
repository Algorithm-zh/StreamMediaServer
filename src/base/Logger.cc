#include "Logger.h"
using namespace tmms::base;

 
Logger::Logger( const FileLogPtr &file_log)  
:log_(file_log){
 
}
void Logger::SetLogLevel(const LogLevel &level)  {
  level_ = level; 
}
 
LogLevel Logger::GetLogLevel() const {
  return level_;
}
 
void Logger::Write(const std::string &msg)  {
  if(log_ != nullptr)
  {
    log_->WriteLog(msg);
  }else{
    std::cout << msg;
  }
}
 
