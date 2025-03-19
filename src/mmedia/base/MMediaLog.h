#pragma once
#include "../../base/LogStream.h"
#include <iostream>

using namespace tmms::base;

#define RTMP_DEBUG_ON 1

#ifdef RTMP_DEBUG_ON
#define RTMP_TRACE std::cout << "\n" 
#define RTMP_DEBUG LOG_DEBUG
#define RTMP_INFO LOG_INFO
#else
#define RTMP_TREACE if(0) LOG_TRACE
#define RTMP_DEBUG if(0) LOG_DEBUG
#define RTMP_INFO if(0) LOG_INFO
#endif

#define RTMP_WARN LOG_WARN
#define RTMP_ERROR LOG_ERROR
