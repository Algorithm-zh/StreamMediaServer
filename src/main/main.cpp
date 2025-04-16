#include "base/Config.h"
#include "base/LogStream.h"
#include "base/FileMgr.h"
#include "base/TaskMgr.h"
#include "live/LiveService.h"
#include <stdio.h>
#include <iostream>
#include <thread>

using namespace tmms::base;
using namespace tmms::live;
using namespace tmms::mm;

int main(int argc,const char ** argv)
{
    g_logger = new Logger(nullptr);
    g_logger->SetLogLevel(kTrace);    
    if(!sConfigMgr->LoadConfig("../config/config.json"))
    {
        std::cerr << "load config file failed." << std::endl;
        return -1;
    }
    ConfigPtr config = sConfigMgr->GetConfig();
    LogInfoPtr log_info = config->GetLogInfo();
    std::cout << "log level:" << log_info->level 
        << " path:" << log_info->path 
        << " name: " << log_info->name 
        << std::endl;
    
    FileLogPtr log = sFileMgr->GetFileLog(log_info->path+log_info->name);
    if(!log)
    {
        std::cerr << "log can't open.exit." << std::endl;
        return -1;
    }
    log->SetRotate(log_info->rotate_type);
    g_logger = new Logger(log);
    g_logger->SetLogLevel(log_info->level);

    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
                                        {
                                            sFileMgr->OnCheck();
                                            task->Restart();
                                        },
                                        1000);      
    sTaskMgr->Add(task4); 
    sLiveService->Start();
    while(1)
    {
        sTaskMgr->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }    
    return 0;
}