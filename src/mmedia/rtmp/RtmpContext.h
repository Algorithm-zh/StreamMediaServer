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
      //发送成员函数
      bool BuildChunk(const PacketPtr &packet, uint32_t timestamp = 0, bool fmt0 = false);
      void Send();
      void CheckAndSend();//检测队列里是不是还有等待发送的，并激活业务层查看是不是还有数据要发
      bool Ready() const;//当前能不能发送数据
      void PushOutQueue(PacketPtr &&packet);
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

      //发送相关
      //核心函数
      bool BuildChunk(PacketPtr &&packet, uint32_t timestamp = 0, bool fmt0 = false);

      char out_buffer_[4096];
      char *out_current_{nullptr};
      std::unordered_map<uint32_t, uint32_t> out_deltas_;
      std::unordered_map<uint32_t, RtmpMsgHeaderPtr> out_message_headers_;
      int32_t out_chunk_size_{4096};
      std::list<PacketPtr> out_waiting_queue_;
      std::list<BufferNodePtr> sending_bufs_;
      std::list<PacketPtr> out_sending_packets_;
      bool sending_{false};
      
    };
    using RtmpContextPtr = std::shared_ptr<RtmpContext>;
  }
}
