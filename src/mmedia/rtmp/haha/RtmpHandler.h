#pragma once
#include "../base/MMediaHandler.h"

namespace tmms
{
  namespace mm
  {
    class RtmpHandler:public MMediaHandler
    {
    public:
      virtual bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param){return false;}
      virtual bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param){return false;}
      virtual void OnPause(const TcpConnectionPtr &conn, bool pause){}
      virtual void OnSeek(const TcpConnectionPtr &conn, double time){}
      virtual void OnPublishPrepare(const TcpConnectionPtr &conn){}
    };
  }
}
