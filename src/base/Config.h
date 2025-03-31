#pragma once
#include <string>
#include "FileLog.h"
#include "Singleton.h"
#include <cstdint>
#include <json/json.h>
#include "NonCopyable.h"
#include "Logger.h"
#include <mutex>
#include <memory>
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
    //配置类
    class Config
    {
    public:
      Config() = default;
      ~Config() = default;

      bool LoadConfig(const std::string &file);
      
      
      LogInfoPtr& GetLogInfo();
      const std::vector<ServiceInfoPtr> & GetServiceInfos();
      const ServiceInfoPtr &GetServiceInfo(const std::string &protocol, const std::string &transport);
      bool ParseServiceInfo(const Json::Value &serviceObj);

      //配置文件信息，后续可以根据配置文件动态更改
      std::string name_;
      int32_t cpu_start_{0};
      int32_t thread_num_{0};
    
    private:
      bool ParseLogInfo(const Json::Value &root);

      LogInfoPtr log_info_;
      std::vector<ServiceInfoPtr> services_;
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


