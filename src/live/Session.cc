#include "Session.h"
#include "Stream.h"
#include "base/TTime.h"
#include "base/AppInfo.h"
#include "user/FlvPlayerUser.h"
#include "live/base/LiveLog.h"
#include "base/StringUtils.h"
#include "user/RtmpPlayerUser.h"

using namespace tmms::live;
using namespace tmms::base;

namespace
{
static UserPtr user_null;
}
Session::Session(const std::string &session_name)
:session_name_(session_name)
{
  stream_ = std::make_shared<Stream>(*this,session_name);
  player_live_time_ = TTime::NowMs();
}

int32_t Session::ReadyTime() const
{
  return stream_->ReadyTime();
}
int64_t Session::SinceStart() const
{
  return stream_->SinceStart();
}
bool Session::IsTimeout()
{
  if(stream_->Timeout())
  {
    return true;
  }
  auto idle = TTime::NowMs() - player_live_time_;
  if(players_.empty()&&idle>app_info_->stream_idle_time)
  {
    return true;
  }
  return false;
}

UserPtr Session::CreatePublishUser(const ConnectionPtr &conn,
                                   const std::string &session_name,
                                   const std::string &param,
                                   UserType type)
{
  if(session_name != session_name_)
  {
    LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
    return user_null;
  }
  auto list = base::StringUtils::SplitString(session_name,"/");
  if(list.size() != 3)
  {
    LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
    return user_null;
  }
  UserPtr user = std::make_shared<User>(conn,stream_,shared_from_this());
  user->SetAppInfo(app_info_);
  user->SetDomainName(list[0]);
  user->SetAppName(list[1]);
  user->SetStreamName(list[2]);
  user->SetParam(param);
  user->SetUserType(type);
  conn->SetContext(kUserContext,user);

  return user;
}
UserPtr Session::CreatePlayerUser(const ConnectionPtr &conn,
                                  const std::string &session_name,
                                  const std::string &param,
                                  UserType type)
{
  if(session_name != session_name_)
  {
    LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
    return user_null;
  }
  auto list = base::StringUtils::SplitString(session_name,"/");
  if(list.size() != 3)
  {
    LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
    return user_null;
  }
  PlayerUserPtr user;
  if(type == UserType::kUserTypePlayerRtmp)
  {
    user = std::make_shared<RtmpPlayerUser>(conn,stream_,shared_from_this());
  }
  else if(type == UserType::kUserTypePlayerFlv)
  {
    user = std::make_shared<FlvPlayerUser>(conn,stream_,shared_from_this());
  }
  else
  {
    return user_null;
  }
  user->SetAppInfo(app_info_);
  user->SetDomainName(list[0]);
  user->SetAppName(list[1]);
  user->SetStreamName(list[2]);
  user->SetParam(param);
  user->SetUserType(type);
  conn->SetContext(kUserContext,user);

  return user;
}
void Session::CloseUser(const UserPtr &user)
{
  if(!user->destroyed_.exchange(true))
  {
    {
      std::lock_guard<std::mutex> lk(lock_);
      if(user->GetUserType() <= UserType::kUserTypePlayerWebRTC)
      {
        if(publisher_)
        {
          LIVE_DEBUG << "remove publisher,session name:" << session_name_
            << ",user:" << user->UserId()
            << ",elapsed:" << user->ElapsedTime()
            << ",ReadyTime:" << ReadyTime()
            << ",stream time:" << SinceStart();

          publisher_.reset();
        }
      }
      else 
    {
        LIVE_DEBUG << "remove player,session name:" << session_name_
          << ",user:" << user->UserId()
          << ",elapsed:" << user->ElapsedTime()
          << ",ReadyTime:" << ReadyTime()
          << ",stream time:" << SinceStart();            
        players_.erase(std::dynamic_pointer_cast<PlayerUser>(user));
        player_live_time_ = tmms::base::TTime::NowMs();
      }
    }
    user->Close();
  }
}

void Session::ActiveAllPlayers()
{
  std::lock_guard<std::mutex> lk(lock_);
  for(auto const &u:players_)
  {
    u->Active();
  }
}   
void Session::AddPlayer(const PlayerUserPtr &user)
{
  {
    std::lock_guard<std::mutex> lk(lock_);
    players_.insert(user);
  }
  LIVE_DEBUG << " add player,session name:" << session_name_ << ",user:" << user->UserId();

  if(!publisher_)
  {

  }
  user->Active();
}
void Session::SetPublisher(UserPtr &user)
{
  std::lock_guard<std::mutex> lk(lock_);
  if(publisher_ == user)
  {
    return;
  }
  if(publisher_&&!publisher_->destroyed_.exchange(true))
  {
    publisher_->Close();
  }
  publisher_ = user;
}

StreamPtr Session::GetStream()
{
  return stream_;
}
const std::string &Session::SessionName()const
{
  return session_name_;
}
void Session::SetAppInfo(AppInfoPtr &ptr)
{
  app_info_ = ptr;
}
AppInfoPtr &Session::GetAppInfo()
{
  return app_info_;
}
bool Session::IsPublishing() const 
{
  return !!publisher_;
}
void Session::Clear()
{
  std::lock_guard<std::mutex> lk(lock_);
  if(publisher_)
  {
    CloseUserNoLock(publisher_);
  }
  for(auto const &p:players_)
  {
    CloseUserNoLock(std::dynamic_pointer_cast<User>(p));
  }
  players_.clear();
}

void Session::CloseUserNoLock(const UserPtr &user)
{
  if(!user->destroyed_.exchange(true))
  {
    {
      if(user->GetUserType() <= UserType::kUserTypePlayerWebRTC)
      {
        if(publisher_)
        {
          LIVE_DEBUG << "remove publisher,session name:" << session_name_
            << ",user:" << user->UserId()
            << ",elapsed:" << user->ElapsedTime()
            << ",ReadyTime:" << ReadyTime()
            << ",stream time:" << SinceStart();
          user->Close();
          publisher_.reset();
        }
      }
      else 
      {
        LIVE_DEBUG << "remove player,session name:" << session_name_
          << ",user:" << user->UserId()
          << ",elapsed:" << user->ElapsedTime()
          << ",ReadyTime:" << ReadyTime()
          << ",stream time:" << SinceStart();            
        players_.erase(std::dynamic_pointer_cast<PlayerUser>(user));
        user->Close();
        player_live_time_ = tmms::base::TTime::NowMs();
      }
    }

  }
}
