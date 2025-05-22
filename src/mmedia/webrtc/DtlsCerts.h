#pragma once
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>

namespace tmms
{
  namespace mm
  {
    class DtlsCerts
    {
    public:
      DtlsCerts() = default;
      ~DtlsCerts();
      bool Init();
      const std::string &Fingerprint() const;
      EVP_PKEY *GetPrivateKey() const;
      X509 *GetCerts() const;
      uint32_t GenRandom();

    private:
      EVP_PKEY * dtls_pkey_{nullptr};
      EVP_PKEY_CTX *pkey_ctx_{nullptr};
      X509 * dtls_certs_{nullptr};
      std::string fingerprint_;//返回sdp时需要把证书指纹传过去，用于校验证书
    };
  }
}
