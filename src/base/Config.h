#pragma once 
#include "json/json.h"
#include "NonCopyable.h"
#include "Singleton.h"
#include "Logger.h"
#include "FileLog.h"
#include <string>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace tmms
{
    namespace base
    {
        using std::string;

        struct LogInfo
        {
            LogLevel level;
            std::string path;
            std::string name;
            RotateType rotate_type{kRotateNone};
        };
        using LogInfoPtr = std::shared_ptr<LogInfo>;

        struct ServiceInfo
        {
            string addr;
            uint16_t port;
            string protocol;
            string transport;
        };
        using ServiceInfoPtr = std::shared_ptr<ServiceInfo>;

        class DomainInfo;
        class AppInfo;
        using DomainInfoPtr = std::shared_ptr<DomainInfo>;
        using AppInfoPtr = std::shared_ptr<AppInfo>;
        class Config
        {
        public:
            Config() = default;
            ~Config() = default;

            bool LoadConfig(const std::string &file);

            LogInfoPtr& GetLogInfo();
            const std::vector<ServiceInfoPtr> & GetServiceInfos();
            const ServiceInfoPtr &GetServiceInfo(const string &protocol,const string &transport);
            bool ParseServiceInfo(const Json::Value &serviceObj);

            AppInfoPtr GetAppInfo(const string &domain,const string &app);
            DomainInfoPtr GetDomainInfo(const string &domain);

            std::string name_;
            int32_t cpu_start_{0};
            int32_t thread_nums_{1};
            int32_t cpus_{1};
        private:
            bool ParseDirectory(const Json::Value &root);
            bool ParseDomainPath(const string &path);
            bool ParseDomainFile(const string &file);

            bool ParseLogInfo(const Json::Value & root);        
            LogInfoPtr log_info_;
            std::vector<ServiceInfoPtr> services_;
            std::unordered_map<std::string,DomainInfoPtr> domaininfos_;
            std::mutex lock_;
        };

        using ConfigPtr = std::shared_ptr<Config>;

        class ConfigMgr:public NonCopyable
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

        #define sConfigMgr tmms::base::Singleton<tmms::base::ConfigMgr>::Instance()
    }
}