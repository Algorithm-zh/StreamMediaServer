#pragma once

#include <cstdint>
#include <string>
#include "mmedia/base/Packet.h"
#include "srtp3/srtp.h"

namespace tmms
{
  namespace mm
  {
    const int32_t kSrtpMaxBufferSize = 65535;
    class Srtp
    {
    public:
      Srtp() = default;
      ~Srtp() = default;
      //初始化
      static bool InitSrtpLibrary();
      bool Init(const std::string &recv_key, const std::string &send_key);
      //加解密成员函数
      PacketPtr RtpProtect(PacketPtr &pkt);
      PacketPtr RtcpProtect(PacketPtr &pkt);
      PacketPtr SrtpUnprotect(PacketPtr &pkt);
      PacketPtr SrtcpUnprotect(PacketPtr &pkt);
    private:
      static void OnSrtpEvent(srtp_event_data_t* data);

      srtp_t send_ctx{nullptr};
      srtp_t recv_ctx{nullptr};
      char w_buffer_[kSrtpMaxBufferSize];
      char r_buffer_[kSrtpMaxBufferSize];
    };
  }
}
