#include "AMFString.h"
#include "../../base/MMediaLog.h"
#include "../../base/BytesReader.h"
using namespace tmms::mm;
 
AMFString::AMFString(const std::string &name)  
:AMFAny(name)
{
}
 
AMFString::AMFString()  {
 
}
 
AMFString::~AMFString()  {
 
}
 
int AMFString::Decode(const char *data, int size, bool has)  {

  if(size < 2)
  {
    return -1;
  }
  auto len = BytesReader::ReadUint16T(data);
  if(len < 0 || size < len + 2)
  {
    //length is not enough
    return -1;
  }
  string_ = DecodeString(data);
  return len + 2;
}
 
bool AMFString::IsString()  {
	return true;
}
 
const std::string &AMFString::String()  {
	return string_;
}
 
void AMFString::Dump() const {
  RTMP_TRACE << "string:" << string_;
}
