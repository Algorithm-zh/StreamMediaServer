#include "RtmpHandShake.h"
#include <cstring>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <random>
#include "../base/MMediaLog.h"
#include "../../base/TTime.h"
#if OPENSSL_VERSION_NUMBER > 0x10100000L
#define HMAC_setup(ctx, key, len) ctx = HMAC_CTX_new(); HMAC_Init_ex(ctx, key, len, EVP_sha256(), NULL)
#define HMAC_crunch(ctx, buf, len) HMAC_Update(ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen) HMAC_Final(ctx, dig, &dlen); HMAC_CTX_free(ctx)
#else
#define HMAC_setup(ctx, key, len) HMAC_CTX_init(&ctx); HMAC_Init_ex(&ctx, key, len, EVP_sha256(), NULL)
#define HMAC_crunch(ctx, buf, len) HMAC_Update(&ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen) HMAC_Final(&ctx, dig, &dlen); HMAC_CTX_cleanup(&ctx)
#endif
namespace 
{
  #define kRtmpHandShakeSize 1536
  static const unsigned char rtmp_server_ver[4] = 
  {
    0x0D, 0x0E, 0x0A, 0x0D
  };
  static const unsigned char rtmp_client_ver[4] = 
  {
    0x0C, 0x00, 0x0D, 0x0E
  };
  //客户端的key
  #define PLAYER_KEY_OPEN_PART_LEN 30
  static const uint8_t rtmp_player_key[] = {
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ', '0', '0', '1',

    0xF0, 0XEE, 0XC2, 0X4A, 0X80, 0X68, 0XBE, 0XE8, 0X2E, 0X00, 0xD0, 0XD1, 0x02,
    0x9E, 0X7E, 0X57, 0X6E, 0XEC, 0X5D, 0X2D, 0X29, 0x80, 0X6F, 0XAB, 0X93, 0XB8,
    0XE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
  };
  //服务端的key
  #define SERVER_KEY_OPEN_PART_LEN 30
  static const uint8_t rtmp_server_key[] = {
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
    'S', 'e', 'r', 'v', 'e', 'r', ' ', '0', '0', '1',

    0xF0, 0XEE, 0XC2, 0X4A, 0X80, 0X68, 0XBE, 0XE8, 0X2E, 0X00, 0xD0, 0XD1, 0x02,
    0x9E, 0X7E, 0X57, 0X6E, 0XEC, 0X5D, 0X2D, 0X29, 0x80, 0X6F, 0XAB, 0X93, 0XB8,
    0XE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
  };
  
  void CalculateDigest(const uint8_t *src, int len, int gap, const uint8_t *key, int keylen, uint8_t *dst)
  {
    uint32_t digestLen = 0;
    #if OPENSSL_VERSION_NUMBER > 0x10100000L
    HMAC_CTX *ctx;
    #else
    HMAC_CTX ctx;
    #endif
    HMAC_setup(ctx, key, keylen);
    if(gap <= 0)
    {
      HMAC_crunch(ctx, src, len);
    }
    else
    {
      HMAC_crunch(ctx, src, gap);
      HMAC_crunch(ctx, src + gap + SHA256_DIGEST_LENGTH, len - gap - SHA256_DIGEST_LENGTH);
    }
    HMAC_finish(ctx, dst, digestLen);
  }
  
  bool VerifyDigest(uint8_t *buf, int digest_pos, const uint8_t *key, size_t keyLen)
  {
    uint8_t digest[SHA256_DIGEST_LENGTH];
    CalculateDigest(buf, kRtmpHandShakeSize, digest_pos, key, keyLen, digest);
    return memcmp(digest, buf + digest_pos + keyLen, SHA256_DIGEST_LENGTH) == 0;
  }

  int32_t GetDigestOffset(const uint8_t *buf, int off, int mod_val)
  {
    uint32_t offset = 0;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(buf + off);
    uint32_t res;
    
    offset = ptr[0] + ptr[1] + ptr[2] + ptr[3];
    res = (offset % mod_val) + (off + 4);
    return res;
  }
}
 
using namespace tmms::mm;

RtmpHandShake::RtmpHandShake(const TcpConnectionPtr &conn, bool client)
:connecion_(conn), is_client_(client) {
 
}
 
void RtmpHandShake::Start()  {
 
}
 
uint8_t RtmpHandShake::GenRandom()  {
  std::mt19937 mt{std::random_device{}()};
  std::uniform_int_distribution<> rand(0, 256);
  return rand(mt) % 256;
}



 
void RtmpHandShake::CreateC1S1()  {
 
  for(int i = 0; i < kRtmpHandShakePacketSize + 1; i++)
  {
    C1S1_[i] = GenRandom();
  }
  //C0S0
  C1S1_[0] = '\x03';
  //timestamp
  memset(C1S1_ + 1, 0x00, 4);

  if(!is_complex_handshake_)
  {
    memset(C1S1_ + 5, 0X00, 4);
  }
  else
  {
    //offset存储的是digest结尾的位置
    //从第8个字节开始，在728个字节中找出digest的位置
    auto offset = GetDigestOffset(C1S1_ + 1, 8, 728);
    //计算摘要
    uint8_t *data = C1S1_  + 1 + offset;
    if(is_client_)
    {
      memcpy(C1S1_ + 5, rtmp_client_ver, 4);
      CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN, data);
    }
    else
    {
      memcpy(C1S1_ + 5, rtmp_server_ver, 4);
      CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN, data);
    }
    memcpy(digest_, data, SHA256_DIGEST_LENGTH);
  }
}
 
int32_t RtmpHandShake::CheckC1S1(const char *data, int bytes)  {
  if(bytes != 1537)
  {
    RTMP_ERROR << "unexpected c1s1 ,len = " << bytes;
    return -1;
  }
  if(data[0] != '\x03')
  {
    RTMP_ERROR << "unexpected c1s1 ,ver = " << data[0];
    return -1;
  }
  //检验摘要
  uint32_t *version = (uint32_t *)(data + 5);
  if(*version == 0)
  {
    //简单握手不需要校验
    is_complex_handshake_ = false;
    return 0;
  }
  int32_t offset = -1;
  if(is_complex_handshake_)
  {
    uint8_t *handshake = (uint8_t *)(data + 1);
    //digest在前的情况
    offset = GetDigestOffset(handshake, 8, 728);
    if(is_client_)
    {
      if(!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN))
      {
        //key在前的情况
        offset = GetDigestOffset(handshake, 772, 728);
        if(!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN))
        {
          return -1;
        }
      }
    }
    else
    {
      if(!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN))
      {
        offset = GetDigestOffset(handshake, 772, 728);
        if(!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN))
        {
          return -1;
        }
      }
    }
  }
  return offset;
}
 
void RtmpHandShake::SendC1S1()  {
  connecion_->Send((const char*)C1S1_, 1537); 
}
 
void RtmpHandShake::CreateC2S2(const char* data, int bytes, int offset)  {
  for(int i = 0; i < kRtmpHandShakePacketSize; i ++) 
  {
    C2S2_[i] = GenRandom();
  }
  memcpy(C2S2_, data, 8);
  auto timestamp = tmms::base::TTime::Now();
  char *t = (char *)&timestamp;
  C2S2_[3] = t[0];
  C2S2_[2] = t[1];
  C2S2_[1] = t[2];
  C2S2_[0] = t[3];
  if(is_complex_handshake_)
  {
    uint8_t digest[32];
    if(is_client_)
    {
      CalculateDigest(digest_, 32, 0, rtmp_player_key, sizeof(rtmp_player_key), digest);
    }
    else
    {
      CalculateDigest(digest_, 32, 0, rtmp_server_key, sizeof(rtmp_server_key), digest);
    }
    CalculateDigest(C2S2_, kRtmpHandShakePacketSize - 32, 0, digest, 32, &C2S2_[kRtmpHandShakePacketSize - 32]);
  }

}
 
void RtmpHandShake::SendC2S2()  {
  connecion_->Send((const char*)C2S2_, kRtmpHandShakePacketSize);
}
 
bool RtmpHandShake::CheckC2S2()  {
  //C2S2可以不要检测
  return true;
}
