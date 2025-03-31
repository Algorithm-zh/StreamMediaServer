#include "AppInfo.h"
#include "LogStream.h"
#include "DomainInfo.h"
using namespace tmms::base;
 
AppInfo::AppInfo(DomainInfo &d)
:domain_info(d)
{
 
}
 
bool AppInfo::ParseAppInfo(Json::Value &root)  {

  Json::Value nameObj = root["name"];
  if(!nameObj.isNull())
  {
    app_name = nameObj.asString();
  }
  Json::Value maxBufferObj = root["max_buffer"];
  if(!maxBufferObj.isNull())
  {
    max_buffer = maxBufferObj.asUInt();
  }
  Json::Value rtmpObj = root["rtmp_support"];
  if(!rtmpObj.isNull())
  {
    rtmp_support = rtmpObj.asString() == "on" ? true : false;
  }
  Json::Value flvObj = root["flv_support"];
  if(!flvObj.isNull())
  {
    flv_support = flvObj.asString() == "on" ? true : false;
  }
  Json::Value hlsObj = root["hls_support"];
  if(!hlsObj.isNull())
  {
    hls_support = hlsObj.asString() == "on" ? true : false;
  }
  Json::Value clObj = root["content_latency"];
  if(!clObj.isNull())
  {
    content_latency = clObj.asUInt();
  }
  Json::Value siObj = root["stream_idle_time"];
  if(!siObj.isNull())
  {
    stream_idle_time = siObj.asUInt();
  }
  Json::Value stObj = root["stream_timeout_time"];
  if(!stObj.isNull())
  {
    stream_timeout_time = stObj.asUInt();
  }
  LOG_INFO << "app name:" << app_name
            << " max buffer:" << max_buffer
            << " rtmp support:" << rtmp_support
            << " flv support:" << flv_support
            << " hls support:" << hls_support
            << " content latency:" << content_latency
            << " stream idle time:" << stream_idle_time
            << " stream timeout time:" << stream_timeout_time;
  return true;
}
