#include "AMFNumber.h"
#include "../../base/MMediaLog.h"
#include "../../base/BytesReader.h"
using namespace tmms::mm;
 
AMFNumber::AMFNumber(const std::string &name)  
:AMFAny(name)
{
}
 
AMFNumber::AMFNumber()  {
 
}
 
AMFNumber::~AMFNumber()  {
 
}
 
int AMFNumber::Decode(const char *data, int size, bool has)  {

  if(size >= 8)
  {
    number_ = BytesReader::ReadUint64T(data); 
    return 8;
  }
  return -1;
}
 
bool AMFNumber::IsNumber()  {
	return true;
}
 
double AMFNumber::Number()  {
	return number_;
}
 
void AMFNumber::Dump() const {
  RTMP_TRACE << "number:" << number_;
}
