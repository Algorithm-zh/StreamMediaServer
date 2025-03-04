#pragma once

#include <functional>
#include <memory>
#include <stdint.h>
#include <unordered_set>
#include <vector>
#include <deque>

namespace tmms
{
  namespace network
  {
    using EntryPtr = std::shared_ptr<void>;
    using WheelEntry = std::unordered_set<EntryPtr>;
    using Wheel = std::deque<WheelEntry>;
    using Wheels = std::vector<Wheel>;
    using Func = std::function<void()>;

    const int kTimingMinute = 60;
    const int kTimingHour = 3600;
    const int kTimingDay = 86400;

    enum TimingType
    {
      kTimingTypeSecond = 0,
      kTimingTypeMinute,
      kTimingTypeHour,
      kTimingTypeDay,
    };

    class CallbackEntry
    {
    public:
      CallbackEntry(const Func &cb) : cb_(cb){};
      ~CallbackEntry()
      {
        if(cb_)
        {
          cb_();
        }
      }
    private:
      Func cb_;
    };
    using CallbackEntryPtr = std::shared_ptr<CallbackEntry>;
    class TimingWheel
    {
    public:
      TimingWheel();
      ~TimingWheel() = default;
      void InsertEntry(uint32_t delay, EntryPtr entryPtr);
      void OnTimer(int64_t now);
      void PopUp(Wheel &bq);
      void RunAfter(double delay, const Func &cb);
      void RunAfter(double delay, Func &&cb);
      void RunEvery(double interval, const Func &cb);
      void RunEvery(double interval, Func &&cb);
    private:
      void InsertSecondEntry(uint32_t delay, EntryPtr entryPtr);
      void InsertMinuteEntry(uint32_t delay, EntryPtr entryPtr);
      void InsertHourEntry(uint32_t delay, EntryPtr entryPtr);
      void InsertDayEntry(uint32_t delay, EntryPtr entryPtr);
      Wheels wheels_;
      int64_t last_ts_{0};//上次时间戳,用于计算当前时间差(单位:ms)
      uint64_t tick_{0};
    };
  }
}
