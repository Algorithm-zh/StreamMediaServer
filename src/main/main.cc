#include "../base/Config.h"
#include "../base/LogStream.h"
#include "../base/FileMgr.h"
#include "../base/TTime.h"
#include "../base/TaskMgr.h"
#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
using namespace tmms::base;
std::thread t;
#if 0
void TestLog()
{
  t = std::thread([](){
    while(true)
    {
      LOG_INFO << "info now :" << TTime::NowMs();
      LOG_DEBUG << "debug now" << TTime::NowMs();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });
}
int main (int argc, char *argv[]) {
  
  if(!sConfigMgr->LoadConfig("../config/config.json"))
  {
    std::cerr << "load config failed" << std::endl;
    return -1;
  }
  ConfigPtr config = sConfigMgr->GetConfig();
  LogInfoPtr log_info = config->GetLogInfo();
  std::cout << "log level:" << log_info->level 
            << "path:" << log_info->path 
            << "name:" << log_info->name << std::endl;
  FileLogPtr log = sFileMgr->GetFileLog(log_info->path + log_info->name);
  if(!log)
  {
    std::cerr << "log can't open" << std::endl;
    return -1;
  }
  log->setRotate(log_info->rotate_type);
  g_logger = new Logger(log);
  g_logger->SetLogLevel(log_info->level);
  TaskPtr task = std::make_shared<Task>([](const TaskPtr &task){
    sFileMgr->OnCheck();
    task->Restart();
  }, 1000);
  TestLog();
  while(1)
  {
    sTaskMgr->OnWork();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return 0;
}
#endif
