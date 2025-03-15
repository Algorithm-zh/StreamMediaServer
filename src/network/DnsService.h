#pragma once
#include "net/EventLoop.h"
#include "base/InetAddress.h"
#include <mutex>
#include <unordered_map>
#include <thread>
#include "../base/NonCopyable.h"

namespace tmms
{
  namespace network
  {
    using InetAddressPtr = std::shared_ptr<InetAddress>;
    class DnsService : public base::NonCopyable
    {
    public:
      DnsService() = default;
      ~DnsService();
      //host相关函数
      void AddHost(const std::string &host);
      InetAddressPtr GetHostAddress(const std::string &host, int index);
      std::vector<InetAddressPtr> GetHostAddress(const std::string &host);
      void UpdateHost(const std::string &host, std::vector<InetAddressPtr> &list);
      std::unordered_map<std::string, std::vector<InetAddressPtr>> GetHosts();
      //其它函数
      void SetDnsServiceParam(int32_t interval, int32_t sleep, int32_t retry);
      void Start();
      void Stop();
      void OnWork();//dns在该函数中解析
      static void GetHostInfo(const std::string &host, std::vector<InetAddressPtr> &list);
    private:
      std::thread thread_;
      bool running_{false};
      std::mutex lock_;
      int32_t interval_{180*1000};//6minus
      int32_t sleep_{200};//ms
      int32_t retry_{3};//try 3 times
      std::unordered_map<std::string, std::vector<InetAddressPtr>> hosts_info_;
      EventLoop *loop_{nullptr};
    };
#define sDnsService tmms::base::Singleton<tmms::network::DnsService>::Instance()
  }
}
