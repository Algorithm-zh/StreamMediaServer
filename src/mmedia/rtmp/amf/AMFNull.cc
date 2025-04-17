#include "AMFNull.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
using namespace tmms::mm;

AMFNull::AMFNull(const std::string &name)
:AMFAny(name)
{

}
AMFNull::AMFNull()
{

}
AMFNull::~AMFNull()
{

}

int AMFNull::Decode(const char *data, int size,bool has)
{
    return 0;
}
bool AMFNull::IsNull()
{
    return true;
}
void AMFNull::Dump() const
{
    RTMP_TRACE << "Null ";
}
