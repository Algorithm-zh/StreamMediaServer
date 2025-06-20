#include "InetAddress.h"
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include "../base/Network.h"
using namespace tmms::network;
 
void InetAddress::GetIpAndPort(const std::string &host, std::string &ip, std::string &port)  {
  
  auto pos = host.find_first_of(':', 0);
  if(pos != std::string::npos)
  {
    ip = host.substr(0, pos);
    port = host.substr(pos + 1);
  }
  else
  {
    ip = host;
  }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool bv6)  
:addr_(ip),port_(std::to_string(port)),is_ipv6_(bv6) {
}

InetAddress::InetAddress(const std::string &host, bool is_v6)  {
  GetIpAndPort(host, addr_, port_);
  is_ipv6_ = is_v6;
}
 
void InetAddress::SetHost(const std::string &host)  {
  GetIpAndPort(host, addr_, port_);
}
 
void InetAddress::SetAddr(const std::string &addr)  {
  addr_ = addr;
}
 
void InetAddress::SetPort(uint16_t port)  {
  port_ = std::to_string(port);
}
 
void InetAddress::SetIsIPV6(bool is_v6)  {
  is_ipv6_ = is_v6;
}
 
const std::string &InetAddress::IP() const {
  return addr_;
}
 
uint32_t InetAddress::IPv4(const char *ip) const {

  struct sockaddr_in addr_in;
  memset(&addr_in, 0x00, sizeof(struct sockaddr_in));
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = 0;
  if(::inet_pton(AF_INET, ip, &addr_in.sin_addr) < 0)
  {
    NETWORK_ERROR << "ipv4 ip:" << ip << " error:" << strerror(errno);
  }
  return ntohl(addr_in.sin_addr.s_addr);
}

uint32_t InetAddress::IPv4() const {
  return IPv4(addr_.c_str());
}
 
std::string InetAddress::ToIpPort() const {
  std::stringstream ss;
  ss << addr_ << ":" << port_;
	return ss.str();
}
 
uint16_t InetAddress::Port() const {
  return std::atoi(port_.c_str());
}
 
void InetAddress::GetSockAddr(struct sockaddr *saddr) const {
  if(IsIpV6())
  {
    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)saddr;
    memset(addr_in6, 0x00, sizeof(struct sockaddr_in6));
    addr_in6->sin6_family = AF_INET6;
    addr_in6->sin6_port = htons(std::atoi(port_.c_str()));
    if(::inet_pton(AF_INET6, addr_.c_str(), &addr_in6->sin6_addr) < 0)
    {
      NETWORK_ERROR << "ipv6 ip:" << addr_ << " error:" << strerror(errno);
    }
    return;
  }

  struct sockaddr_in *addr_in = (struct sockaddr_in *)saddr;
  memset(addr_in, 0x00, sizeof(struct sockaddr_in));
  addr_in->sin_family = AF_INET;
  addr_in->sin_port = htons(std::atoi(port_.c_str()));
  if(::inet_pton(AF_INET, addr_.c_str(), &addr_in->sin_addr) < 0)
  {
    NETWORK_ERROR << "ipv4 ip:" << addr_ << " error:" << strerror(errno);
  }
}
 
bool InetAddress::IsIpV6() const {
	return is_ipv6_;
}
 
bool InetAddress::IsWanIp() const {
  uint32_t a_start = IPv4("10.0.0.0");
  uint32_t a_end = IPv4("10.255.255.255");

  uint32_t b_start = IPv4("172.16.0.0");
  uint32_t b_end = IPv4("172.31.255.255");

  uint32_t c_start = IPv4("192.168.0.0");
  uint32_t c_end = IPv4("192.168.255.255");
  uint32_t ip = IPv4();

  bool is_a = ip >= a_start && ip <= a_end;
  bool is_b = ip >= b_start && ip <= b_end;
  bool is_c = ip >= c_start && ip <= c_end;

  return !is_a && !is_b && !is_c && ip != INADDR_LOOPBACK;
}
 
bool InetAddress::IsLanIp() const {
  uint32_t a_start = IPv4("10.0.0.0");
  uint32_t a_end = IPv4("10.255.255.255");

  uint32_t b_start = IPv4("172.16.0.0");
  uint32_t b_end = IPv4("172.31.255.255");

  uint32_t c_start = IPv4("192.168.0.0");
  uint32_t c_end = IPv4("192.168.255.255");
  uint32_t ip = IPv4();

  bool is_a = ip >= a_start && ip <= a_end;
  bool is_b = ip >= b_start && ip <= b_end;
  bool is_c = ip >= c_start && ip <= c_end;

  return is_a || is_b || is_c;
}
 
bool InetAddress::IsLoopbackIp() const {
  return addr_ == "127.0.0.1";
}
 

