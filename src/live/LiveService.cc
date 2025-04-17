#include "LiveService.h"
#include "base/StringUtils.h"
#include "live/base/LiveLog.h"
#include "mmedia/rtmp/RtmpServer.h"
#include "base/Config.h"
#include "Session.h"
#include "base/TTime.h"
#include "Stream.h"

using namespace tmms::live;
using namespace tmms::mm;

namespace
{
    static SessionPtr session_null;
}
SessionPtr LiveService::CreateSession(const std::string &session_name)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = sessions_.find(session_name);
    if(iter != sessions_.end())
    {
        return iter->second;
    }

    auto list = base::StringUtils::SplitString(session_name,"/");
    if(list.size() != 3)
    {
        LIVE_ERROR << "create session failed. Invalid session name:" << session_name;
        return session_null;
    }
    ConfigPtr config = sConfigMgr->GetConfig();
    auto app_info = config->GetAppInfo(list[0],list[1]);
    if(!app_info)
    {
        LIVE_ERROR << "create session failed. cant found config. domain:" << list[0] << " app:" << list[1];
        return session_null;
    }
    auto s = std::make_shared<Session>(session_name);
    s->SetAppInfo(app_info);

    sessions_.emplace(session_name,s);
    LIVE_DEBUG << "create session success. session_name:" << session_name << " now:" << base::TTime::NowMS();
    return s;
}
SessionPtr LiveService::FindSession(const std::string &session_name)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = sessions_.find(session_name);
    if(iter != sessions_.end())
    {
        return iter->second;
    }
    return session_null;
}
bool LiveService::CloseSession(const std::string &session_name)
{
    SessionPtr s;
    {
        std::lock_guard<std::mutex> lk(lock_);
        auto iter = sessions_.find(session_name);
        if(iter != sessions_.end())
        {
            s = iter->second;
            sessions_.erase(iter);
        }
    }
    if(s)
    {
        LIVE_INFO << " close session:" << s->SessionName()  << " now:" << base::TTime::NowMS();
        s->Clear();
    }
    return true;
}
void LiveService::OnTimer(const TaskPtr &t)
{
    std::lock_guard<std::mutex> lk(lock_);
    for(auto iter = sessions_.begin();iter != sessions_.end();)
    {
        if(iter->second->IsTimeout())
        {
            LIVE_INFO << "session:" << iter->second->SessionName() 
                    << " is timeout. close it. Now:" << base::TTime::NowMS();
            iter->second->Clear();
            iter = sessions_.erase(iter);
        }
        else
        {
            iter ++;
        }
    }
    t->Restart();
}

void LiveService::OnNewConnection(const TcpConnectionPtr &conn)
{

}
void LiveService::OnConnectionDestroy(const TcpConnectionPtr &conn)
{
    auto user = conn->GetContext<User>(kUserContext);
    if(user)
    {
        user->GetSession()->CloseUser(user);
    }
}
void LiveService::OnActive(const ConnectionPtr &conn)
{
    auto user = conn->GetContext<PlayerUser>(kUserContext);
    if(user&&user->GetUserType()>=UserType::kUserTypePlayerPav)
    {
        user->PostFrames();
    }
    // else
    // {
    //     LIVE_ERROR << "no user found.host:" << conn->PeerAddr().ToIpPort();
    //     conn->ForceClose();
    // }
}

bool LiveService::OnPlay(const TcpConnectionPtr &conn,
                        const std::string &session_name, 
                        const std::string &param)
{
    LIVE_DEBUG << "on play session name:" << session_name 
                << " param:" << param 
                << " host:" << conn->PeerAddr().ToIpPort()
                << " now:" << TTime::NowMS();
    auto s = CreateSession(session_name);
    if(!s)
    {
        LIVE_ERROR << "create session failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    } 
    auto user = s->CreatePlayerUser(conn,session_name,param,UserType::kUserTypePlayerRtmp);
    if(!user)
    {
        LIVE_ERROR << "create user failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }     
    conn->SetContext(kUserContext,user);     
    s->AddPlayer(std::dynamic_pointer_cast<PlayerUser>(user));
    return true;     
}
bool LiveService::OnPublish(const TcpConnectionPtr &conn,
                        const std::string &session_name, 
                        const std::string &param)
{
    LIVE_DEBUG << "on publish session name:" << session_name 
                << " param:" << param 
                << " host:" << conn->PeerAddr().ToIpPort()
                << " now:" << TTime::NowMS();
    auto s = CreateSession(session_name);
    if(!s)
    {
        LIVE_ERROR << "create session failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    } 
    auto user = s->CreatePublishUser(conn,session_name,param,UserType::kUserTypePublishRtmp);
    if(!user)
    {
        LIVE_ERROR << "create user failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }     
    conn->SetContext(kUserContext,user);
    s->SetPublisher(user);
    return true;
}
void LiveService::OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) 
{
    auto user = conn->GetContext<User>(kUserContext);
    if(!user)
    {
        LIVE_ERROR << "no found user. host:" << conn->PeerAddr().ToIpPort();
        conn->ForceClose();
        return ;
    }
    user->GetStream()->AddPacket(std::move(data));
}

void LiveService::Start()
{
    ConfigPtr config = sConfigMgr->GetConfig();
    pool_ = new EventLoopThreadPool(config->thread_nums_,config->cpu_start_,config->cpus_);
    pool_->Start();

    // 
    auto services = config->GetServiceInfos();
    auto eventloops = pool_->GetLoops();
    LIVE_TRACE << "eventloops size:" << eventloops.size();
    for(auto &el:eventloops)
    {
        for(auto &s:services)
        {
            if(s->protocol == "RTMP"||s->protocol == "rtmp")
            {
                InetAddress local(s->addr,s->port);
                TcpServer *server = new RtmpServer(el,local,this);
                servers_.push_back(server);
                servers_.back()->Start();
            }
        }
    }
    
    TaskPtr t = std::make_shared<Task>(std::bind(&LiveService::OnTimer,this,std::placeholders::_1),5000);
    sTaskMgr->Add(t);
}

void LiveService::Stop()
{

}
EventLoop *LiveService::GetNextLoop()
{
    return pool_->GetNextLoop();
}

