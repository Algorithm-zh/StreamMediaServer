#pragma once
#include "../base/MMediaHandler.h"

namespace tmms
{
  namespace mm
  {
    class RtmpHandler:public MMediaHandler
    {
    public:
      virtual bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param);
      virtual bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param);
      virtual bool OnPause(const TcpConnectionPtr &conn, bool pause);
      virtual bool OnSeek(const TcpConnectionPtr &conn, double time);
    };
  }
}
