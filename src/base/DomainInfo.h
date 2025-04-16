#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace tmms
{
    namespace base
    {
        using std::string;
        class AppInfo;
        using AppInfoPtr = std::shared_ptr<AppInfo>;

        class DomainInfo
        {
        public:
            DomainInfo() = default;
            ~DomainInfo() = default;

            const string& DomainName() const;
            const string &Type() const;
            bool ParseDomainInfo(const std::string &file);
            AppInfoPtr GetAppInfo(const string &app_name);
        private:
            string name_;
            string type_;
            std::mutex lock_;
            std::unordered_map<string,AppInfoPtr> appinfos_;
        };
    }
}