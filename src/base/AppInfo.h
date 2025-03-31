#pragma once
#include <string>
#include <memory>
#include <json/json.h>

namespace tmms 
{
  namespace base
  {
  
    class DomainInfo;
    using DomainInfoPtr = std::shared_ptr<DomainInfo>;
    class AppInfo
    {
    public:
      explicit AppInfo(DomainInfo &d);
      bool ParseAppInfo(Json::Value &root);
      
      DomainInfo &domain_info;
      std::string domain_name;
      std::string app_name;
      uint32_t max_buffer{1000};//1000帧
      bool rtmp_support{false};
      bool flv_support{false};
      bool hls_support{false};
      uint32_t content_latency{3 * 1000};
      uint32_t stream_idle_time{30 * 1000};
      uint32_t stream_timeout_time{30 * 1000};
      
    private:
    };
  }
}
