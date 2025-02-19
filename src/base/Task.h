#pragma once
#include <cstdint>
#include <functional>
#include <memory>
namespace tmms
{
  namespace base
  {
    //前置声明
    class Task;
    using TaskPtr = std::shared_ptr<Task>;
    using TaskCallback = std::function<void(const TaskPtr &)>;
    //定时任务
    //
    class Task : public std::enable_shared_from_this<Task>
    {
      public:
        Task() = default;
        ~Task() = default;
        Task(const TaskCallback &cb, int64_t interval);
        Task(const TaskCallback &&cb, int64_t interval);
        void Run();
        void Restart();
        int64_t When() const
        {
          return when_;
        }
      private:
        //列表初始化
        int64_t interval_{0};
        int64_t when_{0};
        TaskCallback cb_;
    };
  }
}
