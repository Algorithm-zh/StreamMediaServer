#pragma once
#include <string>
#include "../../network/net/TcpConnection.h"
#include <memory>
#include <cstdint>
#include <openssl/sha.h>
#include <random>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    const int kRtmpHandShakePacketSize = 1536;
    class RtmpHandShake
    {
    public:
      RtmpHandShake(const TcpConnectionPtr &conn, bool client);
      ~RtmpHandShake() = default;
      void Start();
    private:
      uint8_t GenRandom(); 
      //C1S1相关成员函数
      void CreateC1S1();
      //返回大于0表示offset的值，-1表示校验失败，0表示简单模式
      int32_t CheckC1S1(const char *data, int bytes);
      void SendC1S1();
      //C2S2相关成员函数
      //data：s1或c1的握手包，bytes:s1或c1的握手包大小，offset:s1或c1握手包digest位置
      void CreateC2S2(const char* data, int bytes, int offset);
      void SendC2S2();
      bool CheckC2S2();

      TcpConnectionPtr connecion_;
      bool is_client_{false};
      bool is_complex_handshake_{true};
      uint8_t digest_[SHA256_DIGEST_LENGTH];
      //C0S0 and C1S1
      uint8_t C1S1_[kRtmpHandShakePacketSize + 1];
      uint8_t C2S2_[kRtmpHandShakePacketSize];
    };
  }
}
