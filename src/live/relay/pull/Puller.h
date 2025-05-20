#pragma once
#include "network/net/EventLoop.h"
#include "live/Session.h"
#include <cstdint>
namespace tmms
{
  namespace live
  {
    using namespace tmms::network;
    class PullHandler
    {
    public:
        PullHandler(){};
        virtual ~PullHandler(){};
        virtual void OnPullSuccess() = 0;
        virtual void OnPullClose() = 0;
    };
    class Puller
    {
    public:
        Puller(EventLoop *event_loop, Session *s, PullHandler *pull_handler):session_(s),event_loop_(event_loop),pull_handler_(pull_handler){};
        virtual ~Puller(){};
        virtual bool Pull(const TargetPtr &target) = 0;
        EventLoop *GetEventLoop()const
        {
            return event_loop_;
        }
    protected:
        Session *session_{nullptr};
        TargetPtr target_;
        EventLoop *event_loop_{nullptr};
        PullHandler *pull_handler_{nullptr};
    };
  }
}
