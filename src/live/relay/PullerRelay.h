#pragma once
#include "live/relay/pull/Puller.h"
#include "network/net/EventLoop.h"
#include <vector>

namespace tmms
{
  namespace live
  {
    class PullerRelay : public PullHandler
    {
    public:
      PullerRelay(Session &s);
      ~PullerRelay();
      void StartPullStream();//回源入口，启动回源

    private:
      void OnPullSuccess() override;
      void OnPullClose() override;
      bool GetTargets();//获取所有的url
      Puller *GetPuller(TargetPtr p);//根据url创建puller
      void SelectTarget();
      void Pull();
      void ClearPuller();


      Session &session_;
      std::vector<TargetPtr> targets_;
      TargetPtr current_target_;
      int32_t cur_target_index_{-1};
      Puller *puller_{nullptr};
      EventLoop *current_loop_{nullptr};
      
    };
  }
}
