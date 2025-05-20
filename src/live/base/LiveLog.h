#pragma once
#include "base/LogStream.h"
#include <iostream>

using namespace tmms::base;

#define LIVE_DEBUG_ON 1
#define PULLER_DEBUG_ON 1

#ifdef LIVE_DEBUG_ON
#define LIVE_TRACE LOG_TRACE << "LIVE::" 
#define LIVE_DEBUG LOG_DEBUG << "LIVE::"
#define LIVE_INFO LOG_INFO << "LIVE::"
#else
#define LIVE_TREACE if(0) LOG_TRACE
#define LIVE_DEBUG if(0) LOG_DEBUG
#define LIVE_INFO if(0) LOG_INFO
#endif

#define LIVE_WARN LOG_WARN
#define LIVE_ERROR LOG_ERROR


#ifdef PULLER_DEBUG_ON
#define PULLER_TRACE LOG_TRACE << "PULLER::" 
#define PULLER_DEBUG LOG_DEBUG << "PULLER::"
#define PULLER_INFO LOG_INFO << "PULLER::"
#else
#define PULLER_TREACE if(0) LOG_TRACE
#define PULLER_DEBUG if(0) LOG_DEBUG
#define PULLER_INFO if(0) LOG_INFO
#endif

#define PULLER_WARN LOG_WARN
#define PULLER_ERROR LOG_ERROR
