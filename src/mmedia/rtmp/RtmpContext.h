#pragma once
#include "../../network/net/TcpConnection.h"
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include "RtmpHeader.h"
#include "../base/Packet.h"
#include <unordered_map>
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
      ~RtmpContext() = default;
      int32_t Parse(MsgBuffer &buf);
      void OnWriteComplete();
      void StartHandShake();
      //接收成员函数
      int32_t ParseMessage(MsgBuffer &buf);
      void MessageComplete(PacketPtr &&data);
    private:
      RtmpHandShake handshake_;
      int32_t state_{kRtmpHandShake};
      TcpConnectionPtr connection_;
      RtmpHandler *handler_{nullptr};
      std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;
      std::unordered_map<uint32_t, PacketPtr> in_packets_;
      std::unordered_map<uint32_t, uint32_t> in_deltas_;
      std::unordered_map<uint32_t, bool> in_ext_;
      int32_t in_chunk_size_{128};
    };
    using RtmpContextPtr = std::shared_ptr<RtmpContext>;
  }
}
