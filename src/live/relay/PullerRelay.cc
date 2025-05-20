#include "PullerRelay.h"
#include "base/StringUtils.h"
#include "live/base/LiveLog.h"
#include "live/LiveService.h"
#include "base/Target.h"
#include "live/relay/pull/RtmpPuller.h"
#include "live/LiveService.h"
using namespace tmms::live;
 
PullerRelay::PullerRelay(Session &s)
:session_(s)
{
 
}

PullerRelay::~PullerRelay()  {
  ClearPuller(); 
}

void PullerRelay::StartPullStream()  {
  auto ret = GetTargets(); 
  if(!ret)
  {
    PULLER_ERROR << " no target found.";   
    sLiveService->CloseSession(session_.SessionName());
    return ;
  }
  //启动拉流
  Pull();
}
 
void PullerRelay::OnPullSuccess()  {
 
}
 
void PullerRelay::OnPullClose()  {
  Pull(); 
}
 
bool PullerRelay::GetTargets()  {
  auto appinfo = session_.GetAppInfo();
  if(appinfo)
  {
    targets_ = appinfo->pulls;
    if(targets_.size() > 0)
    {
      return true;
    }
  }
  return false;
}
 
Puller PullerRelay::*GetPuller(TargetPtr p)  {
	return nullptr;
}
 
void PullerRelay::SelectTarget()  {
 
  if(current_target_ && current_target_->retry < current_target_->max_retry)
  {
    current_target_->retry ++;
    PULLER_DEBUG << "current target:" << current_target_->session_name 
                 << " index:" << cur_target_index_
                 << " retry:" << current_target_->retry
                 << " max_retry:" << current_target_->max_retry;
    return;
  }
  cur_target_index_ ++;
  if(cur_target_index_ < targets_.size())
  {
    current_target_ = targets_[cur_target_index_];
    ClearPuller();
    if(current_target_->stream_name.empty())
    {
      auto list = base::StringUtils::SplitString(session_.SessionName(), "/");
      if(list.size() == 3)
      {
        current_target_->stream_name = list[2];
      }
    }
    puller_ = GetPuller(current_target_);
    PULLER_ERROR << "select index:" << cur_target_index_;
  }
  else
  {
    PULLER_ERROR << "try all targets, but no stream found.";
    sLiveService->CloseSession(session_.SessionName());
  }
}
 
void PullerRelay::Pull()  {
 
  if(session_.IsPublishing())
  {
    return;
  }
  SelectTarget();
  if(current_target_->retry == 0 || current_target_->interval == 0)
  {
    if(puller_)
    {
      puller_->Pull(current_target_);
    }
  }
  else
  {
    Puller *p = puller_;
    TargetPtr t = current_target_;
    //不是立即拉流，那就启动定时器
    current_loop_->RunAfter(current_target_->interval, [p, t](){
      if(p)
      {
        p->Pull(t);
      }
    });
  }
}
 
void PullerRelay::ClearPuller()  {
 
  if(!puller_)
  {
    return ;
  }
  if(!current_loop_ || current_loop_->IsInLoopThread())
  {
    delete puller_;
    puller_ = nullptr;
  }
  else
  {
    Puller *p = puller_;
    current_loop_->RunInLoop([p](){
      delete p;
    });
    puller_ = nullptr;
  }
}
 

 
Puller *PullerRelay::GetPuller(TargetPtr p)  {

  current_loop_ = sLiveService->GetNextLoop();
  if(p->protocol == "RTMP" || p->protocol == "rtmp")
  {
    return new RtmpPuller(current_loop_, &session_, this);
  }
  return nullptr;
}
