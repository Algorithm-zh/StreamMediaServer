#pragma once
#include <cstdint>
#include <openssl/types.h>
#include <string>
#include "DtlsHandler.h"
#include "mmedia/webrtc/DtlsCerts.h"
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
namespace tmms
{
  namespace mm
  {
    class Dtls
    {
    public:
      Dtls(DtlsHandler *handler);
      ~Dtls();
      //初始化成员函数
      bool Init();

      void OnRecv(const char *data, uint32_t size);
      //其他成员函数
      const std::string &Fingerprint() const;
      void SetDone();
      void SetClient(bool client);
      const std::string &SendKey();
      const std::string &RecvKey();

    private:
      bool InitSSLContext();
      bool InitSSL();
      //ssl操作成员函数
      static int SSLVerify(int preverify_ok, X509_STORE_CTX *ctx);
      static void SSLInfo(const SSL* ssl, int where, int ret);
      void NeedPost();
      // 此函数实现了 RFC 5764 中规定的 DTLS-SRTP 密钥派生过程，从 DTLS 握手过程中导出密钥材料用于保护 SRTP 媒体流。
      // 通过分离发送和接收密钥确保每个通信方向都有自己的加密材料，而客户端/服务器的区分确保密钥被正确分配到相应的角色。
      void GetSrtpKey();

      SSL_CTX *ssl_context_{nullptr};
      DtlsCerts dtls_certs_;
      bool is_client_{false};
      bool is_done_{false};
      SSL *ssl_{nullptr};
      BIO *bio_read_{nullptr};
      BIO *bio_write_{nullptr};
      char buffer_[65535];
      DtlsHandler *handler_{nullptr};
      std::string send_key_;
      std::string recv_key_;
    };
  }
}
