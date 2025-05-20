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
    enum RtmpHandShakeState
    {
      kHandShakeInit,
      //client
      kHandShakePostC0C1,
      kHandShakeWaitS0S1,
      kHandShakePostC2,
      kHandShakeWaitS2,
      kHandShakeDoning,

      //server
      kHandShakeWaitC0C1,
      kHandShakePostS0S1,
      kHandShakePostS2,
      kHandShakeWaitC2,

      kHandShakeDone,
    };
    class RtmpHandShake
    {
    public:
      RtmpHandShake(const TcpConnectionPtr &conn, bool client = false);
      ~RtmpHandShake() = default;
      void Start();
      //状态机函数
      //返回值0握手成功 1需要更多数据 2正在完成握手 -1出错
      int32_t HandShake(MsgBuffer &buf);
      void WriteComplete();
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
      bool CheckC2S2(const char *data,int bytes);

      TcpConnectionPtr connection_;
      bool is_client_{false};
      bool is_complex_handshake_{true};
      uint8_t digest_[SHA256_DIGEST_LENGTH];
      //C0S0 and C1S1
      uint8_t C1S1_[kRtmpHandShakePacketSize + 1];
      uint8_t C2S2_[kRtmpHandShakePacketSize];
      int32_t state_{kHandShakeInit};
    };
  }
}
