#pragma once

#include "NonCopyable.h"
#include "Task.h"
#include "Singleton.h"
#include <cstdint>
#include <mutex>
#include <unordered_set>
namespace tmms 
{
  namespace base
  {
    class TaskMgr : public NonCopyable
    {
      public:
        TaskMgr()=default;
        ~TaskMgr()=default;
        
        //遍历执行任务
        void OnWork();
        bool Add(TaskPtr &task);
        bool Del(TaskPtr &task);
      private:
        //智能指针的hash使用的是裸指针的值计算的，
  //    //因此所有指向同一个裸指针的智能指针对应的hash值是相同的
        std::unordered_set<TaskPtr> tasks_; 
        std::mutex lock_;
    };
  }
  #define sTaskMgr tmms::base::Singleton<tmms::base::TaskMgr>::Instance()
}
