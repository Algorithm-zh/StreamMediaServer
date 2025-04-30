#pragma once
#include "../../base/LogStream.h"
#include <iostream>

using namespace tmms::base;

#define RTMP_DEBUG_ON 1
#define HTTP_DEBUG_ON 1
#define DEMUX_DEBUG_ON 1

#ifdef RTMP_DEBUG_ON
#define RTMP_TRACE LOG_TRACE << "RTMP::" 
#define RTMP_DEBUG LOG_DEBUG << "RTMP::"
#define RTMP_INFO LOG_INFO << "RTMP::"
#else
#define RTMP_TREACE if(0) LOG_TRACE
#define RTMP_DEBUG if(0) LOG_DEBUG
#define RTMP_INFO if(0) LOG_INFO
#endif

#define RTMP_WARN LOG_WARN
#define RTMP_ERROR LOG_ERROR


#ifdef HTTP_DEBUG_ON
#define HTTP_TRACE LOG_TRACE << "HTTP::" 
#define HTTP_DEBUG LOG_DEBUG << "HTTP::"
#define HTTP_INFO LOG_INFO << "HTTP::"
#else
#define HTTP_TREACE if(0) LOG_TRACE
#define HTTP_DEBUG if(0) LOG_DEBUG
#define HTTP_INFO if(0) LOG_INFO
#endif

#define HTTP_WARN LOG_WARN
#define HTTP_ERROR LOG_ERROR



#ifdef DEMUX_DEBUG_ON
#define DEMUX_TRACE LOG_TRACE << "DEMUX::" 
#define DEMUX_DEBUG LOG_DEBUG << "DEMUX::"
#define DEMUX_INFO LOG_INFO << "DEMUX::"
#else
#define DEMUX_TREACE if(0) LOG_TRACE
#define DEMUX_DEBUG if(0) LOG_DEBUG
#define DEMUX_INFO if(0) LOG_INFO
#endif

#define DEMUX_WARN LOG_WARN
#define DEMUX_ERROR LOG_ERROR
