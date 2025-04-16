#include "AMFBoolean.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
using namespace tmms::mm;

AMFBoolean::AMFBoolean(const std::string &name)
:AMFAny(name)
{

}
AMFBoolean::AMFBoolean()
{

}
AMFBoolean::~AMFBoolean()
{

}

int AMFBoolean::Decode(const char *data, int size,bool has)
{
    if(size >= 1)
    {
        b_ = *data!=0?true:false;
        return 1;
    }
    return -1;
}
bool AMFBoolean::IsBoolean()
{
    return true;
}
bool AMFBoolean::Boolean()
{
    return b_;
}
void AMFBoolean::Dump() const
{
    RTMP_TRACE << "Boolean: " << b_;
}