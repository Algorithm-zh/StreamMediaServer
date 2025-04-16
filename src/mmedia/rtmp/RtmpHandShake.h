#pragma once

#include "network/net/TcpConnection.h"
#include <string>
#include <memory>
#include <cstdint>
#include <openssl/sha.h>
#include <openssl/hmac.h>

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
            RtmpHandShake(const TcpConnectionPtr &conn,bool client=false);
            ~RtmpHandShake() = default;
            void Start();

            int32_t HandShake(MsgBuffer &buf);
            void WriteComplete();
        private:
            uint8_t GenRandom();
            void CreateC1S1();
            int32_t CheckC1S1(const char *data,int bytes);
            void SendC1S1();
            void CreateC2S2(const char *data, int bytes,int offset);
            void SendC2S2();
            bool CheckC2S2(const char *data,int bytes);

            TcpConnectionPtr connection_;
            bool is_client_{false};
            bool is_complex_handshake_{true};
            uint8_t digest_[SHA256_DIGEST_LENGTH];
            uint8_t C1S1_[kRtmpHandShakePacketSize+1];
            uint8_t C2S2_[kRtmpHandShakePacketSize];
            int32_t state_{kHandShakeInit};
        };
        using RtmpHandShakePtr =std::shared_ptr<RtmpHandShake>;
    }
}