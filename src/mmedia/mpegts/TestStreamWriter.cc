#include "TestStreamWriter.h"
#include "mmedia/base/MMediaLog.h"
#include <fcntl.h>
#include <unistd.h>
using namespace tmms::mm;
 
TestStreamWriter::TestStreamWriter()  {
 
  //O_TRUNC是创建文件时将文件的内容全部删除
  fd_ = ::open("test.ts", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_ < 0)
  {
    MPEGTS_DEBUG << "open test.ts failed.error:" << errno;
  }
}
 
TestStreamWriter::~TestStreamWriter()  {
 
  if(fd_ >= 0)
  {
    ::close(fd_);
    fd_ = -1;
  }
}
 
int32_t TestStreamWriter::Write(void *buf, uint32_t size)  {

  if(fd_ >= 0)
  {
    auto ret = ::write(fd_, buf, size);
    if(ret != size)
    {
      MPEGTS_WARN << "write ret :" << ret << " size:" << size;
    }
  }
  return size;
}
