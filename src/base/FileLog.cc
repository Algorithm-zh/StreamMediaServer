#include "FileLog.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
using namespace tmms::base;

 
bool FileLog::Open(const std::string &filePath)  {
  file_path_ = filePath;
  //DEFFILEMODE表示文件所有者可以读写文件，而其他用户只能读取文件
  int fd = ::open(filePath.c_str(), O_CREAT | O_APPEND | O_WRONLY, DEFFILEMODE);
  if(fd < 0)
  {
    std::cout << "open file error" << filePath << std::endl;
    return false;
  }
  fd_ = fd;
	return true;
}
 
size_t FileLog::WriteLog(const std::string &msg)  {
  int fd = fd_ == -1 ? 1 : fd_;
  //原子操作,不会在下面写完之前被关闭
  return ::write(fd_, msg.data(), msg.size());
}
 
void FileLog::Rotate(const std::string &file)  {
  if(file_path_.empty())
  {
    return ;
  }
  int ret = ::rename(file_path_.c_str(), file.c_str());
  if(ret != 0)std::cerr << "rename file error" << file << std::endl;
  //有append,保证了原子操作，所以dup2关闭fd_时保证了关闭和写不会冲突
  int fd = ::open(file_path_.c_str(), O_CREAT | O_APPEND | O_WRONLY, DEFFILEMODE);
  if(fd < 0)
  {
    std::cout << "open file error" << file << std::endl;
    return ;
  }
  //复制文件描述符,并且关闭fd_所指的文件
  ::dup2(fd, fd_);
  close(fd);
}

void FileLog::setRotate(RotateType type)  {
  rotate_type_ = type;
}
 
RotateType FileLog::GetRotateType() const {
	return rotate_type_;
}
 
int64_t FileLog::FileSize() const {
  return ::lseek64(fd_, 0, SEEK_END);
}
 
std::string FileLog::FilePath() const {
	return file_path_;
}
