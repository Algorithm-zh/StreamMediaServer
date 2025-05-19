#pragma once
#include "json/value.h"

namespace tmms
{
  namespace base
  {
    class Target
    {
    public:
      Target(const std::string &s);
      Target(const std::string &domain_name, const std::string &app_name);
      ~Target() = default;
      bool ParseTarget(Json::Value &root);
      void ParseTargetUrl(const std::string &url);
      std::string remote_host;
      unsigned short remote_port;
      std::string session_name;
      std::string domain_name;
      std::string app_name;
      std::string stream_name;
      std::string protocol;
      std::string url;
      int32_t interval{1000};
      int32_t max_retry = 0;
      int32_t retry = 0;
      std::string param;
    };
  }
}

