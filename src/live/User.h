#pragma once

#include "network/net/Connection.h"
#include "base/AppInfo.h"
#include <memory>
#include <cstdint>
#include <string>
#include <atomic>

namespace tmms
{
    namespace live
    {
        using namespace tmms::network;
        using namespace tmms::base;
        using std::string;
        using AppInfoPtr = std::shared_ptr<AppInfo>;
        class Session;
        using SessionPtr = std::shared_ptr<Session>;
        enum class UserType
        {
            kUserTypePublishRtmp = 0,
            kUserTypePublishMpegts ,
            kUserTypePublishPav,
            kUserTypePublishWebRtc ,
            kUserTypePlayerPav ,
            kUserTypePlayerFlv ,
            kUserTypePlayerHls ,
            kUserTypePlayerRtmp ,
            kUserTypePlayerWebRTC ,
            kUserTypeUnknowed = 255,
        };

        enum class UserProtocol
        {
            kUserProtocolHttp = 0,
            kUserProtocolHttps ,
            kUserProtocolQuic ,
            kUserProtocolRtsp ,
            kUserProtocolWebRTC ,
            kUserProtocolUdp ,
            kUserProtocolUnknowed = 255
        };
        class Stream;
        using StreamPtr = std::shared_ptr<Stream>;

        class User:public std::enable_shared_from_this<User>
        {
        public:
            friend class Session;
            explicit User(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);
            virtual ~User() = default;

            const string &DomainName() const ;
            void SetDomainName(const string &domain);
            const string &AppName() const ;
            void SetAppName(const string &domain);
            const string &StreamName() const ;
            void SetStreamName(const string &domain);
            const string &Param() const ;
            void SetParam(const string &domain);   
            const AppInfoPtr &GetAppInfo()const;
            void SetAppInfo(const AppInfoPtr &info);
            virtual UserType GetUserType() const;
            void SetUserType(UserType t);
            virtual UserProtocol GetUserProtocol() const ;
            void SetUserProtocol(UserProtocol p) ;
            
            void Close();
            ConnectionPtr GetConnection();
            uint64_t ElapsedTime();
            void Active();
            void Deactive();
            const std::string &UserId() const 
            {
                return user_id_;
            }
            SessionPtr GetSession() const 
            {
                return session_;
            }
            StreamPtr GetStream() const 
            {
                return stream_;
            }
        protected:
            ConnectionPtr connection_;
            StreamPtr stream_;
            string domain_name_;
            string app_name_;
            string stream_name_;
            string param_;
            string user_id_;
            AppInfoPtr app_info_;
            int64_t start_timestamp_{0};
            UserType type_{UserType::kUserTypeUnknowed};
            UserProtocol protocol_{UserProtocol::kUserProtocolUnknowed};
            std::atomic_bool destroyed_{false};
            SessionPtr session_;
        };
    }
}