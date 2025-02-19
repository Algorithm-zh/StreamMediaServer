#pragma once
#include "NonCopyable.h"
#include <iostream>

namespace tmms {
  namespace base {
    enum LogLevel {
      kTrace = 0,
      kDebug,
      kInfo,
      kWarn,
      kError,
      KMaxNumOfLogLevel,
    };
    class Logger : public NonCopyable {
    public:
      Logger() = default;
      ~Logger() = default;
      
      void SetLogLevel(const LogLevel &level);
      LogLevel GetLogLevel() const;
      void Write(const std::string &msg);
    private:
      LogLevel level_{kDebug};
    };
  }
}
