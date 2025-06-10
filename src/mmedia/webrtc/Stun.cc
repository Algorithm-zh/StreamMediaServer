#include "Stun.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
#include "mmedia/base/BytesWriter.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/RtmpHandShake.h"
#include "mmedia/mpegts/TsTool.h"
#include <iostream>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/core_names.h> 
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/types.h>
using namespace tmms::mm;
 
bool Stun::Decode(const char* data, uint32_t size)  {
  //messagetype
  type_ = (StunMessageType)BytesReader::ReadUint16T(data);
  data += 2;
  stun_length_ = BytesReader::ReadUint16T(data);
  data += 2;
  //magiccookie
  auto magic = BytesReader::ReadUint32T(data);
  if(magic != kStunMagicCookie)
  {
    WEBRTC_ERROR << "stun magic error" << " not equal " << kStunMagicCookie;
    return false;
  }
  data += 4;

  transaction_id_.assign(data, 12);
  data += 12;
  WEBRTC_DEBUG << "stun type:" << type_ 
                << " length:" << stun_length_ 
                << " transaction:" << transaction_id_;
  size -= 20;
  //解析属性
  while(size >= 4)
  {
    uint16_t attr_type = BytesReader::ReadUint16T(data);
    data += 2;
    uint16_t attr_len = BytesReader::ReadUint16T(data);
    data += 2;
    size -= 4;

    uint16_t padding_len = (4 - attr_len % 4) % 4;
    if(size < padding_len + attr_len)
    {
      WEBRTC_ERROR << "stun attr length:" << attr_len
                    << " padding length:" << padding_len
                    << " size:" << size;
      return false;
    }
    switch (attr_type) {
      case kStunAttrUsername:
      {
        user_name_.assign(data, attr_len);
        WEBRTC_DEBUG << "username:" << user_name_;
        break;
      }
      case kStunAttrPassword:
      {
        password_.assign(data, attr_len);
        WEBRTC_DEBUG << "password:" << password_;
        break;
      }
    }
    data += (attr_len + padding_len);
    size -= (attr_len + padding_len);
  }
  return true;
}
 
PacketPtr Stun::Encode()  {
  PacketPtr packet = Packet::NewPacket(512);
  char *data = packet->Data();
  data += BytesWriter::WriteUint16T(data, type_);
  data += BytesWriter::WriteUint16T(data, 0);
  data += BytesWriter::WriteUint32T(data, kStunMagicCookie);
  std::memcpy(data, transaction_id_.c_str(), transaction_id_.size());
  data += 12;

  //username
  //计算padding(4字节对齐)
  int32_t padding_bytes = (4 - user_name_.size() % 4) % 4;
  BytesWriter::WriteUint16T(data, kStunAttrUsername);
  BytesWriter::WriteUint16T(data + 2, user_name_.size());
  std::memcpy(data + 4, user_name_.c_str(), user_name_.size());
  if(padding_bytes != 0)
  {
      std::memset(data + 4 + user_name_.size(), 0, padding_bytes);
  }
  data += (4 + user_name_.size() + padding_bytes);

  //mapaddress
  BytesWriter::WriteUint16T(data, kStunAttrXorMappedAddress);
  BytesWriter::WriteUint16T(data + 2, 8);//属性长度
  BytesWriter::WriteUint8T(data + 4, 0);//保留位
  BytesWriter::WriteUint8T(data + 5, 0x01);//协议族
  BytesWriter::WriteUint16T(data + 6, mapped_port_ ^ (kStunMagicCookie >> 16));
  BytesWriter::WriteUint32T(data + 8, mapped_addr_ ^ kStunMagicCookie);
  data += (4 + 8);

  //integrity(消息完整性)
  char *data_begin = packet->Data();
  size_t data_bytes = data - data_begin - 20;
  // 初始长度 = 数据长度 + MESSAGE-INTEGRITY 属性头（4字节） + HMAC值（20字节）
  size_t paylod_len = data_bytes + (4 + 20);//没有加上fingerprint的长度
  BytesWriter::WriteUint16T(data_begin + 2, paylod_len);
  BytesWriter::WriteUint16T(data, kStunAttrMessageIntegrity);
  BytesWriter::WriteUint16T(data + 2, 20);
  CalcHmac(data + 4, data_begin, data_bytes + 20);//20个字节的摘要
  //计算完成后，恢复实际长度
  paylod_len = data_bytes + (4 + 20) + (4 + 4);
  BytesWriter::WriteUint16T(data_begin + 2, paylod_len);
  data += (4 + 20);
  //finger
  BytesWriter::WriteUint16T(data, kStunAttrFingerprint);
  BytesWriter::WriteUint16T(data + 2, 4);
  uint32_t crc32 = TsTool::CRC32Ieee(packet->Data(), data - packet->Data()) ^ 0x5354554e;
  BytesWriter::WriteUint32T(data + 4, crc32);
  std::cout << crc32 << std::endl;
  data += (4 + 4);

  packet->SetPacketSize(paylod_len + 20);
  return packet;
}

 
std::string Stun::LocalUFrag()  {
  //前面local后面remote
  auto pos = user_name_.find_first_of(':');
  if(pos != std::string::npos)
  {
    return user_name_.substr(0, pos);
  }
  return std::string();
}
 
void Stun::SetPassword(const std::string &pwd)  {
  password_ = pwd;
}
 
void Stun::SetMappedAddr(uint32_t addr)  {
  mapped_addr_ = addr;
}
 
void Stun::SetMappedPort(uint16_t port)  {
  mapped_port_ = port;
}
 
void Stun::SetMessageType(StunMessageType type)  {
  type_ = type;
}
 
//size_t Stun::CalcHmac(char *buf, const char *data, size_t bytes)
//{
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//    unsigned int digestLen;
//#if OPENSSL_VERSION_NUMBER > 0x10100000L
//    HMAC_CTX *ctx = HMAC_CTX_new();
//    HMAC_Init_ex(ctx, password_.c_str(), password_.size(), EVP_sha1(), NULL);
//    HMAC_Update(ctx, (const unsigned char *)data, bytes);
//    HMAC_Final(ctx, (unsigned char *)buf, &digestLen);
//    HMAC_CTX_free(ctx);
//#else
//    HMAC_CTX ctx;
//    HMAC_CTX_init(&ctx);
//    HMAC_Init_ex(&ctx, password_.c_str(), password_.size(), EVP_sha1(), NULL);
//    HMAC_Update(&ctx, (const unsigned char *)data, bytes);
//    HMAC_Final(&ctx, (unsigned char *)buf, &digestLen);
//    HMAC_CTX_cleanup(&ctx);
//#endif
//    return digestLen;
//}
size_t Stun::CalcHmac(char *buf, const char *data, size_t bytes) {
    size_t digestLen = 0;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // OpenSSL 3.0+ 推荐方式（EVP_MAC + SHA1）
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (!mac) return 0;

    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);
    if (!ctx) {
        EVP_MAC_free(mac);
        return 0;
    }

    // 配置 HMAC 使用 SHA1
    OSSL_PARAM params[2] = {
        OSSL_PARAM_construct_utf8_string("digest", (char *)"SHA1", 0),
        OSSL_PARAM_construct_end()
    };

    // 初始化、更新、计算 HMAC
    if (EVP_MAC_init(ctx, (const unsigned char*)password_.c_str(), password_.size(), params) &&
        EVP_MAC_update(ctx, (const unsigned char*)data, bytes) &&
        EVP_MAC_final(ctx, (unsigned char*)buf, &digestLen, EVP_MAX_MD_SIZE)) {
        // 成功
    } else {
        digestLen = 0; // 失败
    }

    // 释放资源
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
#else
    // OpenSSL 1.1.x 及以下（传统 HMAC_CTX）
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, (const unsigned char*)password_.c_str(), password_.size(), EVP_sha1(), NULL);
    HMAC_Update(&ctx, (const unsigned char*)data, bytes);
    HMAC_Final(&ctx, (unsigned char*)buf, &digestLen);
    HMAC_CTX_cleanup(&ctx);
#endif

    return digestLen;
}

