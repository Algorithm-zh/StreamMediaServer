#include "AMFLongString.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
using namespace tmms::mm;

AMFLongString::AMFLongString(const std::string &name)
:AMFAny(name)
{

}
AMFLongString::AMFLongString()
{

}
AMFLongString::~AMFLongString()
{

}

int AMFLongString::Decode(const char *data, int size,bool has)
{
    if(size < 4)
    {
        return -1;
    }
    auto len = BytesReader::ReadUint32T(data);
    if(len<0 || size < len+4)
    {
        return -1;
    }
    string_.assign(data+4,len);
    return len + 4;
}
bool AMFLongString::IsString()
{
    return true;
}
const std::string& AMFLongString::String()
{
    return string_;
}
void AMFLongString::Dump() const
{
    RTMP_TRACE << "LongString: " << string_;
}