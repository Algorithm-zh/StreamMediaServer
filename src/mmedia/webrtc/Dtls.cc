#include "Dtls.h"
#include "mmedia/base/MMediaLog.h"
#include <openssl/bio.h>
#include <openssl/srtp.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
using namespace tmms::mm;
 

Dtls::Dtls(DtlsHandler *handler)
:handler_(handler)
{
 
}

Dtls::~Dtls()  {
  if(ssl_context_) 
  {
    SSL_CTX_free(ssl_context_);
    ssl_context_ = nullptr;
  }
  if(ssl_)
  {
    SSL_free(ssl_);
    ssl_ = nullptr;
    //bio
    bio_read_ = nullptr;
    bio_write_ = nullptr;
  }
    
}
 
bool Dtls::Init()  {
  auto ret = dtls_certs_.Init();
  if(!ret)
  {
    return false;
  }
  ret = InitSSLContext();
  if(!ret)
  {
    return false;
  }
  ret = InitSSL();
  return true;
}
 
void Dtls::OnRecv(const char *data, uint32_t size)  {
  
  BIO_reset(bio_read_);
  BIO_reset(bio_write_);

  BIO_write(bio_read_, data, size);
  SSL_do_handshake(ssl_);

  NeedPost();
  if(is_done_)
  {
    GetSrtpKey();
    return;
  }
  SSL_read(ssl_, buffer_, 65535);
}
 
const std::string &Dtls::Fingerprint() const {
  return dtls_certs_.Fingerprint();
}
 
void Dtls::SetDone()  {
  is_done_ = true; 
}
 
void Dtls::SetClient(bool client)  {
  is_client_ = true; 
}
 
const std::string &Dtls::SendKey()  {
  return send_key_;
}
 
const std::string &Dtls::RecvKey()  {
  return recv_key_;
}
 
bool Dtls::InitSSLContext()  {

  ssl_context_ = SSL_CTX_new(DTLS_method()); 
  SSL_CTX_use_certificate(ssl_context_, dtls_certs_.GetCerts());
  auto ret = SSL_CTX_use_PrivateKey(ssl_context_, dtls_certs_.GetPrivateKey());
  if(!ret)
  {
    WEBRTC_ERROR << "SSL_CTX_use_PrivateKey failed";
    return false;
  }
  ret = SSL_CTX_check_private_key(ssl_context_);
  if(!ret)
  {
    WEBRTC_ERROR << "SSL_CTX_check_private_key failed";
    return false;
  }
  SSL_CTX_set_cipher_list(ssl_context_, "ALL");
  SSL_CTX_set_verify(ssl_context_, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, Dtls::SSLVerify);
  SSL_CTX_set_info_callback(ssl_context_, Dtls::SSLInfo);
  
  SSL_CTX_set_verify_depth(ssl_context_, 4);
  SSL_CTX_set_read_ahead(ssl_context_, 1);
  SSL_CTX_set_tlsext_use_srtp(ssl_context_, "SRTP_AES128_CM_SHA1_80");
  return true;
}

 
bool Dtls::InitSSL()  {

  ssl_ = SSL_new(ssl_context_);
  if(!ssl_)
  {
    WEBRTC_ERROR << "SSL_new failed";
    return false;
  }
  SSL_set_ex_data(ssl_, 0, static_cast<void*>(this));

  bio_read_ = BIO_new(BIO_s_mem());
  bio_write_ = BIO_new(BIO_s_mem());
  SSL_set_bio(ssl_, bio_read_, bio_write_);
  
  SSL_set_mtu(ssl_, 1350);
  SSL_set_accept_state(ssl_);
  return true;
}
 
int Dtls::SSLVerify(int preverify_ok, X509_STORE_CTX *ctx)  {
	return 1;
}
 
void Dtls::SSLInfo(const SSL* ssl, int where, int ret)  {
  Dtls *dtls = static_cast<Dtls*>(SSL_get_ex_data(ssl, 0));
  int w = where & ~SSL_ST_MASK;
  //判断客户端还是服务端
  if(w & SSL_ST_CONNECT)
  {
      dtls->SetClient(true);
  }
  else if(w & SSL_ST_ACCEPT)
  {
      dtls->SetClient(false);
  }
  else
  {
      dtls->SetClient(false);
  }
  if(where & SSL_CB_HANDSHAKE_DONE)
  {
      dtls->SetDone();
      WEBRTC_TRACE << "dtls handshake done.";
  }
}
 
void Dtls::NeedPost()  {
 
  //end. no data
  if(BIO_eof(bio_write_))
  {
    return ;   
  }
  char *data = nullptr;
  auto read = BIO_get_mem_data(bio_write_, &data);
  if(read <= 0)
  {
    return ;
  }

  if(handler_)
  {
    handler_->OnDtlsSend(data, read, this);
  }
  BIO_reset(bio_write_);
}
 
void Dtls::GetSrtpKey()  {
  int32_t srtp_key_len = 16; 
  int32_t srtp_salt_len = 14;
  unsigned char material[30*2];
  static std::string label = "EXTRACTOR-dtls_srtp";
  auto ret = SSL_export_keying_material(ssl_, material, sizeof(material), label.c_str(), label.size(), NULL, 0, 0);
  if(ret <= 0)
  {
    WEBRTC_ERROR << "SSL_export_keying_material failed:" << ret;
  }
  int32_t offset = 0;
  std::string client_master_key((char*)material, srtp_key_len);
  offset += srtp_key_len;
  std::string server_master_key((char*)(material + offset), srtp_key_len);
  offset += srtp_key_len;
  std::string client_master_salt((char*)(material + offset), srtp_salt_len);
  offset += srtp_salt_len;
  std::string server_master_salt((char*)(material + offset), srtp_salt_len);
  offset += srtp_salt_len;

  if(is_client_)
  {
    recv_key_ = server_master_key + server_master_salt;
    send_key_ = client_master_key + client_master_salt;
  }
  else
  {
    recv_key_ = client_master_key + client_master_salt;
    send_key_ = server_master_key + server_master_salt;
  }
}
 
