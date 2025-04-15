#pragma once
#include <string>
#include "FileLog.h"
#include "Singleton.h"
#include <cstdint>
#include <json/json.h>
#include "NonCopyable.h"
#include "Logger.h"
#include "base/AppInfo.h"
#include "base/DomainInfo.h"
#include <mutex>
#include <memory>
#include <unordered_map>
#include <vector>

namespace tmms 
{
  namespace base 
  {
    struct LogInfo
    {
      //日志级别
      LogLevel level;
      //日志文件路径
      std::string path;
      //日志文件名
      std::string name;
      //日志文件切分类型
      RotateType rotate_type{kRotateNone};
    };
    using LogInfoPtr = std::shared_ptr<LogInfo>;
    struct ServiceInfo
    {
        std::string addr;
        uint16_t port;
        std::string protocol;
        std::string transport;
    };
    using ServiceInfoPtr = std::shared_ptr<ServiceInfo>;
    class DomainInfo;
    class AppInfo;
    using DomainInfoPtr = std::shared_ptr<DomainInfo>;
    using AppInfoPtr = std::shared_ptr<AppInfo>;
    //配置类
    class Config
    {
    public:
      Config() = default;
      ~Config() = default;

      bool LoadConfig(const std::string &file);
      
      
      LogInfoPtr& GetLogInfo();
      //获取服务信息
      const std::vector<ServiceInfoPtr> & GetServiceInfos();
      const ServiceInfoPtr &GetServiceInfo(const std::string &protocol, const std::string &transport);
      bool ParseServiceInfo(const Json::Value &serviceObj);
      //获取应用信息
      AppInfoPtr GetAppInfo(const std::string &domain, const std::string &app);
      DomainInfoPtr GetDomainInfo(const std::string &domain);
      //void SetDomainInfo(const std::string &domain, DomainInfoPtr &p);
      //void SetAppInfo(const std::string &domain, const std::string &app);

      //配置文件信息，后续可以根据配置文件动态更改
      std::string name_;
      int32_t cpu_start_{0};
      int32_t thread_num_{1};
      int32_t cpus_{1};
    
    private:
      //解析配置文件
      bool ParseDirectory(const Json::Value &root);
      bool ParseDomainPath(const std::string &path);
      bool ParseDomainFile(const std::string &file);
      //解析日志配置
      bool ParseLogInfo(const Json::Value &root);

      LogInfoPtr log_info_;
      std::vector<ServiceInfoPtr> services_;
      std::unordered_map<std::string,  DomainInfoPtr> domaininfos_;
      std::mutex lock_;
    };
    using ConfigPtr = std::shared_ptr<Config>;
    class ConfigMgr : public NonCopyable
    {
    public:
      ConfigMgr() = default;
      ~ConfigMgr() = default;
      bool LoadConfig(const std::string &file);
      ConfigPtr GetConfig();
    
    private:
      ConfigPtr config_; 
      std::mutex lock_;
    };
  }
}

#define sConfigMgr tmms::base::Singleton<tmms::base::ConfigMgr>::Instance()


