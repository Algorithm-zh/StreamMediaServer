#include "Config.h"
#include "LogStream.h"
#include "json/reader.h"
#include <fstream>
#include <memory>
#include <mutex>
using namespace tmms::base;

namespace {
  static ServiceInfoPtr service_info_nullptr;
}
 
bool Config::LoadConfig(const std::string &file)  {

  LOG_DEBUG << "CONFIG FILE: " << file;
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::ifstream in(file);
  std::string err;
  auto ok = Json::parseFromStream(reader, in, &root, &err);
  if(!ok) {
    LOG_ERROR << "CONFIG LOAD FAILED" << file << "parse error";
    return false;
  }

  Json::Value nameObj = root["name"];
  if(!nameObj.isNull()) {
    name_ = nameObj.asString();
  }
  Json::Value cpuStartObj = root["cpu_start"];
  if(!cpuStartObj.isNull()) {
    cpu_start_ = cpuStartObj.asInt();
  }
  Json::Value threadNumObj = root["threads"];
  if(!threadNumObj.isNull()) {
    thread_num_ = threadNumObj.asInt();
  }
  Json::Value logObj = root["log"];
  if(!logObj.isNull()){
    ParseLogInfo(logObj);
  }
  if(!ParseServiceInfo(root["services"]))
  {
    return false;
  }

  return true;
}
 
bool Config::ParseLogInfo(const Json::Value &root)  {

  log_info_ = std::make_shared<LogInfo>();
  //打印级别
  Json::Value levelObj = root["level"];
  if(!levelObj.isNull()) {
    std::string level = levelObj.asString();
    if(level == "TRACE")
    {
      log_info_->level = kTrace;
    }
    else if(level == "DEBUG")
    {
      log_info_->level = kDebug;
    }
    else if(level == "INFO")
    {
      log_info_->level = kInfo;
    }
    else if(level == "WARN")
    {
      log_info_->level = kInfo;
    }
    else if(level == "ERROR")
    {
      log_info_->level = kError;
    }
  }
  //切分条件处理
  Json::Value rtObj = root["rotate"];
  if(!rtObj.isNull()) {
    std::string rt = rtObj.asString();
    if(rt == "DAY")
    {
      log_info_->rotate_type = kRotateDay;
    }
    else if(rt == "HOUR")
    {
      log_info_->rotate_type = kRotateHour;
    }
  }
 
  Json::Value pathObj = root["path"];
  if(!pathObj.isNull()) {
    log_info_->path = pathObj.asString();
  }
  Json::Value nameObj = root["name"];
  if(!nameObj.isNull()) {
    log_info_->name = nameObj.asString();
  }
	return true;
}
 
LogInfoPtr& Config::GetLogInfo()  {
  return log_info_;
}
 

bool ConfigMgr::LoadConfig(const std::string &file)  {

  LOG_DEBUG << "load config file:" << file;
  ConfigPtr config = std::make_shared<Config>();
  if(config->LoadConfig(file)) {
    config_ = config;
    return true;
  }
  return false;
}
 
ConfigPtr ConfigMgr::GetConfig()  {
  std::lock_guard<std::mutex> lk(lock_);
  return config_;
}
 
const std::vector<ServiceInfoPtr> &Config::GetServiceInfos()  {
  return services_;
}
 
const ServiceInfoPtr &Config::GetServiceInfo(const std::string &protocol, const std::string &transport)  {
  for(auto &s : services_)
  {
    if(s->protocol == protocol && s->transport == transport)
    {
      return s;
    }
  }
  return service_info_nullptr;
}
 
bool Config::ParseServiceInfo(const Json::Value &serviceObj)  {
  
  if(serviceObj.isNull())
  {
    LOG_ERROR << "config no service section!";
    return false;
  }
  if(!serviceObj.isArray())
  {
    LOG_ERROR << "config service section is not array!";
    return false;
  }
  for(auto const &s : serviceObj)
  {
    ServiceInfoPtr sinfo = std::make_shared<ServiceInfo>();
    sinfo->addr = s.get("addr", "0.0.0.0").asString();
    sinfo->port = s.get("port", "0").asInt();
    sinfo->protocol = s.get("protocol", "rtmp").asString();
    sinfo->transport = s.get("transport", "tcp").asString();

    LOG_INFO << "service info addr:" << sinfo->addr << ", port:" << sinfo->port 
             << ", protocol:" << sinfo->protocol << ", transport:" << sinfo->transport;
    services_.emplace_back(sinfo);
  }
  return true;
}
