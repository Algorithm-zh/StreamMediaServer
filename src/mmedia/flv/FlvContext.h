#pragma once
#include "network/net/Connection.h"
#include "network/net/TcpConnection.h"
#include "mmedia/base/Packet.h"
#include "mmedia/base/MMediaHandler.h"
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    class FlvContext : public MMediaHandler
    {
    public:
      FlvContext(const TcpConnectionPtr &conn, MMediaHandler *handler);
      ~FlvContext() = default;
      void SendFlvHttpHeader(bool has_video, bool has_audio);
      void WriteFlvHeader(bool has_video, bool has_audio);
      bool BuildFlvFrame(PacketPtr &pkt, uint32_t timestamp);
      void Send();
      void WriteComplete(const TcpConnectionPtr &conn);
      bool Ready() const;
    private:
      char GetRtmpPacketType(PacketPtr &pkt);//返回包类型
      std::list<BufferNodePtr> bufs_;
      std::list<PacketPtr> out_packets_;
      TcpConnectionPtr connection_;
      uint32_t previous_size_{0};
      std::string http_header_;
      char out_buffer_[512];
      char *current_{nullptr};
      bool sending_{false};
      MMediaHandler *handler_{nullptr};
    };
  }
}
