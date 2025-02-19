#include "Task.h"
#include "TTime.h"
#include <algorithm>
using namespace tmms::base;

Task::Task(const TaskCallback &cb, int64_t interval)
  :interval_(interval), cb_(cb)
{
  when_ = TTime::NowMs() + interval_;
}

Task::Task(const TaskCallback &&cb, int64_t interval)
  :interval_(interval), cb_(std::move(cb))
{
  when_ = TTime::NowMs() + interval_;
}

void Task::Run()
{
  if (cb_)
  {
    cb_(shared_from_this()); 
  }
}
void Task::Restart()
{
  when_ = interval_ + TTime::NowMs();
}
