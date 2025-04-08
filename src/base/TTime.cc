#include "TTime.h"
using namespace tmms::base;
int64_t TTime::NowMs()  {
  struct timeval tv;
  //获取当前精准时间
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
 
int64_t TTime::Now()  {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}
 
int64_t TTime::Now(int &year, int &month, int &day, int &hour, int &minute, int &second)  {
  struct tm tm;
  time_t t = time(NULL);
  //把时间转换为本地时间
  localtime_r(&t, &tm);
  year = tm.tm_year + 1900;
  month = tm.tm_mon + 1;
  day = tm.tm_mday;
  hour = tm.tm_hour;
  minute = tm.tm_min;
  second = tm.tm_sec;
  return t;
}
 
std::string TTime::ISOTime()  {

  //这俩没用到
  struct timeval tv;
  gettimeofday(&tv, NULL);//能获得从epoch到当前时间总微秒数
  
  struct tm tm;
  time_t t = time(NULL);//获取从epoch到当前时间总秒数
  localtime_r(&t, &tm);//通过时间总秒数获取对应UTC标准时间
  char buf[128] = {0};
  auto n = sprintf(buf, "%4d-%02d-%02dT%02d:%02d:%02d",
                   tm.tm_year + 1900,
                   tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return std::string(buf, buf + n);
}
