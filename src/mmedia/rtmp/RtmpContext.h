#pragma once
#include "../../network/net/TcpConnection.h"
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    enum RtmpContextState
    {
      kRtmpHandShake = 0,
      kRtmpWaitingDone,
      kRtmpMessage,
    };
    class RtmpContext
    {
    public:
      //握手相关成员函数
      RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client = false);
      int32_t Parse(MsgBuffer &buf);
      void OnWriteComplete();
      void StartHandShake();
      ~RtmpContext() = default;
    private:
      RtmpHandShake handshake_;
      int32_t state_{kRtmpHandShake};
      TcpConnectionPtr connection_;
      RtmpHandler *handler_{nullptr};
    };
  }
}
