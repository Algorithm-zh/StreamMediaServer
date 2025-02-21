#include "../base/Logger.h"
#include "../base/LogStream.h"
#include "../base/FileMgr.h"
#include <thread>
#include "../base/TTime.h"
#include "../base/TaskMgr.h"
using namespace tmms::base;

std::thread t;
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
int main()
{
  FileLogPtr log = sFileMgr->GetFileLog("test.log");
  log->setRotate(kRotateMinute);
  g_logger = new Logger(log);
  g_logger->SetLogLevel(kTrace);
  TaskPtr task = std::make_shared<Task>([](const TaskPtr &task){
    sFileMgr->OnCheck();
    task->Restart();
  }, 1000);
  sTaskMgr->Add(task);
  TestLog();
  while(1)
  {
    sTaskMgr->OnWork();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return 0;
}
