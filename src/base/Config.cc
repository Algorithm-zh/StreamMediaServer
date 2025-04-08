#include "Config.h"
#include "LogStream.h"
#include "json/reader.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <memory>
#include <mutex>
#include <dirent.h>
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
  ParseDirectory(root["directory"]);

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

//解析目录
bool Config::ParseDirectory(const Json::Value &root)  {
  if(root.isNull() || !root.isArray())
  {
    return false;
  }
  for(const Json::Value &d : root)
  {
    std::string path = d.asString();
    struct stat st;

    int ret = stat(path.c_str(), &st);
    LOG_TRACE << "ret: " << ret << "errno: " << errno;
    if(ret != -1)
    {
      if((st.st_mode & S_IFMT) == S_IFDIR)
      {
        LOG_TRACE << "dir:" << path;
        ParseDomainPath(path);
      }
      else if((st.st_mode & S_IFMT) == S_IFREG)
      {
        ParseDomainFile(path);
      }
    }
    LOG_TRACE << "path:" << path;
  }
  return true;
}

//解析目录下的文件
bool Config::ParseDomainPath(const std::string &path)  {

  DIR *dp = nullptr;
  struct dirent *pp = nullptr;
  LOG_DEBUG << "parse domain path:" << path;
  dp = opendir(path.c_str());
  if(dp == nullptr)
  {
    return false;
  }
  while(true)
  {
    pp = readdir(dp);
    if(pp == nullptr)
    {
      break;
    }
    //过滤文件(隐藏文件，当前目录 和上级目录)
    if(pp->d_name[0] == '.')
    {
      continue;
    }
    if(pp->d_type == DT_REG)
    {
      //转绝对路径
      if(path.at(path.size() - 1) != '/')
      {
        ParseDomainFile(path + "/" + pp->d_name);
      }
      else
      {
        ParseDomainFile(path + pp->d_name);
      }
    }
      
  }
  closedir(dp);
  return true;
}
 
//解析文件
bool Config::ParseDomainFile(const std::string &file)  {
  LOG_DEBUG << "parse domain file:" << file;
  DomainInfoPtr d = std::make_shared<DomainInfo>();
  auto ret = d->ParseDomainInfo(file);
  if(ret)
  {
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = domaininfos_.find(d->DomainName());
    if(iter != domaininfos_.end())
    {
      domaininfos_.erase(iter);
    }
    domaininfos_.emplace(d->DomainName(), d);
  }
  return true;
}
 

 
AppInfoPtr Config::GetAppInfo(const std::string &domain, const std::string &app)  {

  std::lock_guard<std::mutex> lk(lock_);
  auto iter = domaininfos_.find(domain);
  if(iter != domaininfos_.end())
  {
    return iter->second->GetAppInfo(app);
  }
  return AppInfoPtr();
}
 
DomainInfoPtr Config::GetDomainInfo(const std::string &domain)  {

  std::lock_guard<std::mutex> lk(lock_);
  auto iter = domaininfos_.find(domain);
  if(iter != domaininfos_.end())
  {
    return iter->second;
  }
  return DomainInfoPtr();
}
