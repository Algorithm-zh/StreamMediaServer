#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace tmms 
{
  namespace base
  {
    class AppInfo;
    using AppInfoPtr = std::shared_ptr<AppInfo>;
    class DomainInfo
    {
    public:
      DomainInfo() = default;
      ~DomainInfo() = default;
      const std::string& DomainName() const;
      const std::string& Type() const;
      bool ParseDomainInfo(const std::string &file);
      AppInfoPtr GetAppInfo(const std::string &app_name);

    private:
      std::string name_;
      std::string type_;
      std::mutex lock_;
      std::unordered_map<std::string, AppInfoPtr> appinfos_;
    };
  }

}
