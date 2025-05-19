#include "User.h"
#include "base/TTime.h"
#include "live/Stream.h"
using namespace tmms::live;

 
User::User(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s)
:connection_(ptr), stream_(stream), session_(s){
  start_timestamp_ = tmms::base::TTime::NowMs(); 
  user_id_ = ptr->PeerAddr().ToIpPort();
}
 
const std::string &User::DomainName() const {
  return domain_name_;
}
 
void User::SetDomainName(const std::string &domain_name)  {
  domain_name_ = domain_name;
}
 
const std::string &User::AppName() const {
  return app_name_;
}
 
void User::SetAppName(const std::string &app_name)  {
  app_name_ = app_name;
}
 
const std::string &User::StreamName() const {
  return stream_name_;
}
 
void User::SetStreamName(const std::string &stream_name)  {
  stream_name_ = stream_name;
}
 
const std::string &User::Param() const {
  return param_;
}
 
void User::SetParam(const std::string &param)  {
  param_ = param;
}
 
const AppInfoPtr &User::GetAppInfo() const {
  return app_info_;
}
 
void User::SetAppInfo(const AppInfoPtr &app_info)  {
  app_info_ = app_info;
}
 
UserType User::GetUserType() const {
	return type_;
}
 
void User::SetUserType(UserType type)  {
  type_ = type;
}
 
UserProtocol User::GetUserProtocol() const {
	return protocol_;
}
 
void User::SetUserProtocol(UserProtocol protocol)  {
  protocol_ = protocol;
}
 
void User::Close()  {
  if(connection_) 
  {
    connection_->ForceClose();
  }
}
 
ConnectionPtr User::GetConnection()  {
  return connection_;
}
 
uint64_t User::ElapsedTime()  {
  return tmms::base::TTime::NowMs() - start_timestamp_;
}
 
void User::Active()  {
   
  if(connection_) 
  {
    connection_->Active();
  }
}
 
void User::Deactive()  {
   
  if(connection_) 
  {
    connection_->Deactive();
  }
}
