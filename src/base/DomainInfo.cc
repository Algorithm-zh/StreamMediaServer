#include "DomainInfo.h"
#include "LogStream.h"
#include <fstream>
#include <json/json.h>
#include "AppInfo.h"

using namespace tmms::base;

 
const std::string& DomainInfo::DomainName() const {
  return name_;
}
 
const std::string& DomainInfo::Type() const {
  return type_;
}
 
bool DomainInfo::ParseDomainInfo(const std::string &file)  {
  LOG_DEBUG << "parse domain info file:" << file;
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::ifstream in(file);
  std::string err;
  auto ok = Json::parseFromStream(reader, in, &root, &err);
  if(!ok) {
    LOG_ERROR << "CONFIG LOAD FAILED" << file << "parse error";
    return false;
  }

  Json::Value domainObj = root["domain"];
  if(domainObj.isNull()) {
    LOG_ERROR << "CONFIG LOAD FAILED" << file << "domain section is null";
    return false;
  }
  Json::Value nameObj = domainObj["name"];
  if(!nameObj.isNull()) {
    name_ = nameObj.asString();
  }
  Json::Value typeObj = domainObj["type"];
  if(!typeObj.isNull()) {
    type_ = typeObj.asString();
  }
  Json::Value appsObj = domainObj["app"];
  if(appsObj.isNull()) {
    LOG_ERROR << "CONFIG LOAD FAILED" << file << "app section is null";
    return false;
  }
  for(auto &aObj : appsObj)
  {
    AppInfoPtr appinfo = std::make_shared<AppInfo>(*this);
    auto ret = appinfo->ParseAppInfo(aObj);
    if(ret)
    {
      std::lock_guard<std::mutex> lk(lock_);
      appinfos_.emplace(appinfo->app_name, appinfo);
    }
  }
  return true;
}
 
AppInfoPtr DomainInfo::GetAppInfo(const std::string &app_name)  {
  std::lock_guard<std::mutex> lk(lock_);
  auto it = appinfos_.find(app_name);
  if(it != appinfos_.end())
  {
    return it->second;
  }
  return AppInfoPtr();
}
