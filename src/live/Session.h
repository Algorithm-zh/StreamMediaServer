#pragma once

#include "PlayerUser.h"
#include "User.h"
#include "base/AppInfo.h"
#include "network/net/Connection.h"
#include <atomic>
#include <unordered_set>
namespace tmms
{
  namespace live
  {
    using PlayerUserPtr = std::shared_ptr<PlayerUser>;
    using UserPtr = std::shared_ptr<User>;
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
      explicit Session(const std::string &session_name);
      //时间处理成员函数
      int32_t ReadyTime() const;
      int64_t SinceStart() const;
      bool IsTimeout();
      //用户创建关闭成员函数
      UserPtr CreatePublishUser(const ConnectionPtr &conn, 
                                const std::string &session_name, 
                                const std::string &param,
                                UserType type);
      UserPtr CreatePlayerUser(const ConnectionPtr &conn, 
                               const std::string &session_name, 
                               const std::string &param,
                               UserType type);
      void CloseUser(const UserPtr &user);
      //用户操作成员函数
      void ActiveAllPlayers();
      void AddPlayer(const PlayerUserPtr &user);
      void SetPublisher(UserPtr &user);
      //其它成员函数
      StreamPtr GetStream();
      const std::string &SessionName() const;
      void SetAppInfo(AppInfoPtr &ptr);
      AppInfoPtr &GetAppInfo();
      bool IsPublishing() const;
      void Clear();
    private:
      void CloseUserNoLock(const UserPtr &user);
      std::string session_name_;
      std::unordered_set<PlayerUserPtr> players_;
      AppInfoPtr app_info_;
      StreamPtr stream_;
      UserPtr publisher_;
      std::mutex lock_;
      std::atomic_int64_t player_live_time_;
    };
  }
}
