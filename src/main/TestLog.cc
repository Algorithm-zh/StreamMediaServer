#include "../base/Logger.h"
#include "../base/LogStream.h"
using namespace tmms::base;
void TestLog()
{
  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "info";
  LOG_WARN << "warn";
  LOG_ERROR << "error";
}
int main()
{
  g_logger = new Logger();
  g_logger->SetLogLevel(kTrace);
  TestLog();
  return 0;
}
