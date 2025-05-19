#pragma once
#include "base/Config.h"
#include "network/net/Connection.h"
#include "base/AppInfo.h"
#include <memory>
#include <cstdint>
#include <string>

namespace tmms
{
  namespace live
  {
    using namespace tmms::network;
    using namespace tmms::base;
    using AppInfoPtr = std::shared_ptr<AppInfo>;
    class Session;
    using SessionPtr = std::shared_ptr<Session>;
    enum class UserType
    {
        kUserTypePublishRtmp = 0,
        kUserTypePublishMpegts,
        kUserTypePublishPav,
        kUserTypePublishWebRtc,
        kUserTypePlayerPav,
        kUserTypePlayerFlv,
        kUserTypePlayerHls,
        kUserTypePlayerRtmp,
        kUserTypePlayerWebRTC,
        kUserTypeUnknowed = 255,
    };
    enum class UserProtocol
    {
        kUserProtocolHttp = 0,
        kUserProtocolHttps,
        kUserProtocolQuic,
        kUserProtocolRtsp,
        kUserProtocolWebRTC,
        kUserProtocolUdp,
        kUserProtocolUnknowed = 255,
    };
    class Stream;
    using StreamPtr = std::shared_ptr<Stream>;
    class User : public std::enable_shared_from_this<User>
    {
    public:
      friend class Session;
      explicit User(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s);
      virtual ~User() = default;
      //SessionName成员函数
      const std::string &DomainName() const;
      void SetDomainName(const std::string &domain_name);
      const std::string &AppName() const;
      void SetAppName(const std::string &app_name);
      const std::string &StreamName() const;
      void SetStreamName(const std::string &stream_name);
      const std::string &Param() const;
      void SetParam(const std::string &param);
      //配置信息成员函数
      const AppInfoPtr &GetAppInfo() const;
      void SetAppInfo(const AppInfoPtr &app_info);
      //User类型协议成员函数
      virtual UserType GetUserType() const;
      void SetUserType(UserType type);
      virtual UserProtocol GetUserProtocol() const;
      void SetUserProtocol(UserProtocol protocol);
      //other
      void Close();
      ConnectionPtr GetConnection();
      uint64_t ElapsedTime();//用户创建了多长时间
      void Active();//激活用户，有数据请求时会调用
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
      std::string domain_name_;
      std::string app_name_;
      std::string stream_name_;
      std::string param_;
      std::string user_id_;
      AppInfoPtr app_info_;
      int64_t start_timestamp_{0};
      UserType type_{UserType::kUserTypeUnknowed};
      UserProtocol protocol_{UserProtocol::kUserProtocolUnknowed};
      std::atomic_bool destroyed_{false};
      SessionPtr session_;
    };
  }
}
