#include "TaskMgr.h"
#include "TTime.h"
#include <cstdint>
using namespace tmms::base;

 
void TaskMgr::OnWork()  {
 
  std::lock_guard<std::mutex> lock(lock_);
  int64_t now = TTime::NowMs();
  for(auto iter = tasks_.begin(); iter != tasks_.end();)
  {
    if((*iter)->When() < now)
    {
      (*iter)->Run();
      //看看该任务是否重置时间了，如果没有就删除
      if((*iter)->When() < now)
      {
        //删除并返回下一个迭代器
        iter = tasks_.erase(iter);
        continue;
      }
    }
    iter++;
  }
}
 
bool TaskMgr::Add(TaskPtr &task)  {
  std::lock_guard<std::mutex> lock(lock_);
  auto iter = tasks_.find(task);
  if(iter != tasks_.end())
  {
    return false;
  }
  tasks_.emplace(task);
  return true;
}
 
bool TaskMgr::Del(TaskPtr &task)  {
  std::lock_guard<std::mutex> lock(lock_);
  tasks_.erase(task);
  return true;
}
