#include "LogStream.h"
#include <string.h>
#include "TTime.h"
#include <unistd.h>
#include <syscall.h>
using namespace tmms::base;
Logger *tmms::base::g_logger;
//thread_local,一个线程单独拥有一个，与其他线程互不影响
//pid_t is int
static thread_local pid_t thread_id = 0;
const char *log_string[] = {
  " TRACE ",
  " DEBUG ",
  " INFO ",
  " WARN ",
  " ERROR ",
};
LogStream::LogStream(Logger *loger, const char * file, int line, LogLevel l, const char *func)  
:logger_(loger){
  const char* file_name = strrchr(file, '/');
  if(file_name)
  {
    file_name = file_name + 1;
  }
  else {
    file_name = file;
  }
  stream_ << TTime::ISOTime();
  if(thread_id == 0)
  {
    thread_id = static_cast<pid_t>(syscall(SYS_gettid));
  }
  stream_ << thread_id;
  stream_ << log_string[l];
  stream_ << "[" << file_name << ":" << line << "]";
  if(func)
  {
    stream_ << "[" << func << "]";
  }
}
 
 
LogStream::~LogStream()  {
  stream_ << "\n";
  if(logger_)
    logger_->Write(stream_.str());
  else
   std::cout << stream_.str() << std::endl;
}
