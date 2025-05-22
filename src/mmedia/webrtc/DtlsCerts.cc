#include "DtlsCerts.h"
#include "mmedia/base/MMediaLog.h"
#include <random>
#include <openssl/asn1.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/x509.h>
using namespace tmms::mm;
 

bool DtlsCerts::Init()  {

  //生成dtls密钥
  pkey_ctx_ = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
  if(!pkey_ctx_)
  {
    WEBRTC_ERROR << "EVP_PKEY_CTX_new_id failed";
    return false;
  }
  auto ret = EVP_PKEY_keygen_init(pkey_ctx_);
  if(ret <= 0)
  {
    WEBRTC_ERROR << "EVP_PKEY_keygen_init failed";
    return false;
  }
  ret = EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pkey_ctx_, NID_X9_62_prime256v1);
  if(ret <= 0)
  {
    WEBRTC_ERROR << "EVP_PKEY_CTX_set_ec_paramgen_curve_nid failed";
    return false;
  }
  ret = EVP_PKEY_CTX_set_ec_param_enc(pkey_ctx_, OPENSSL_EC_NAMED_CURVE);
  if(ret <= 0)
  {
    WEBRTC_ERROR << "EVP_PKEY_CTX_set_ec_param_enc failed";
    return false;
  }
  ret = EVP_PKEY_keygen(pkey_ctx_, &dtls_pkey_);
  if(ret <= 0)
  {
    WEBRTC_ERROR << "EC_KEY_generate_key failed";
    return false;
  }

  //生成dtls证书
  dtls_certs_ = X509_new();
  //设置随机数
  auto req = GenRandom();
  ASN1_INTEGER_set(X509_get_serialNumber(dtls_certs_), req);
  //设置证书有效期
  X509_gmtime_adj(X509_get_notBefore(dtls_certs_), 0);
  X509_gmtime_adj(X509_get_notAfter(dtls_certs_), 60*60*24*365);//一年
  
  //设置证书专有属性
  X509_NAME *subject = X509_NAME_new();
  const std::string &aor = "tmms.net" + std::to_string(req);
  X509_NAME_add_entry_by_txt(subject, "O", MBSTRING_ASC, (const unsigned char*)aor.c_str(), aor.size(), -1, 0);
  //设置别名
  const std::string &aor1 = "tmms.cn";
  X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC, (const unsigned char*)aor1.c_str(), aor1.size(), -1, 0);

  X509_set_issuer_name(dtls_certs_, subject);
  X509_set_subject_name(dtls_certs_, subject);

  //设置公钥
  X509_set_pubkey(dtls_certs_, dtls_pkey_);
  X509_sign(dtls_certs_, dtls_pkey_, EVP_sha1());

  //计算指纹
  unsigned char fingerprint_bin[EVP_MAX_MD_SIZE];
  unsigned int len = 0;
  X509_digest(dtls_certs_, EVP_sha256(), fingerprint_bin, &len);

  char fingerprint_result[EVP_MAX_MD_SIZE * 3 + 1];
  for(int i = 0; i < len; i++)
  {
    sprintf(fingerprint_result + (i * 3), "%.2X:", fingerprint_bin[i]);
  }
  //去掉最后一个冒号
  if(len > 0)
  {
    fingerprint_result[len * 3 - 1] = 0;
  }
  fingerprint_.assign(fingerprint_result, len * 3 - 1);
  WEBRTC_DEBUG << "fingerprint:" << fingerprint_;
  return true;
}
 
const std::string &DtlsCerts::Fingerprint() const {
  return fingerprint_;
}
 
EVP_PKEY *DtlsCerts::GetPrivateKey() const {
  return dtls_pkey_;
}
 
X509 *DtlsCerts::GetCerts() const {
  return dtls_certs_;
}
 
uint32_t DtlsCerts::GenRandom()  {
  std::mt19937 mt{std::random_device{}()};
  return mt();
}
 
DtlsCerts::~DtlsCerts()  {
 
  //释放密钥和证书
  if(dtls_pkey_)
  {
    EVP_PKEY_free(dtls_pkey_);
    dtls_pkey_ = nullptr;
  }
  if(dtls_certs_)
  {
    X509_free(dtls_certs_);
    dtls_certs_ = nullptr;
  }
}
