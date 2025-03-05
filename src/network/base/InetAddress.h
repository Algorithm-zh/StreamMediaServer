#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bits/socket.h>
#include <string>

namespace tmms
{
  namespace network
  {
    class InetAddress
    {
    public:
      static void GetIpAndPort(const std::string &host, std::string &ip, std::string &port);
      //构造函数
      InetAddress(const std::string &ip, uint16_t port, bool bv6=false);
      InetAddress(const std::string &host, bool is_v6=false);
      InetAddress()=default;
      ~InetAddress()=default;
      //赋值函数
      void SetHost(const std::string &host);//ip和端口都在一个url里
      void SetAddr(const std::string &addr);
      void SetPort(uint16_t port);
      void SetIsIPV6(bool is_v6);
      //取值函数
      const std::string &IP() const;
      uint32_t IPv4() const;
      std::string ToIpPort() const;//ip:port
      uint16_t Port() const;
      void GetSockAddr(struct sockaddr *saddr) const;//用于网络连接
      //测试函数
      bool IsIpV6() const;
      bool IsWanIp() const;
      bool IsLanIp() const;
      bool IsLoopbackIp() const;
    private:
      uint32_t IPv4(const char *ip) const;
      std::string addr_;
      std::string port_;
      bool is_ipv6_{false};
    };
  }   
}
