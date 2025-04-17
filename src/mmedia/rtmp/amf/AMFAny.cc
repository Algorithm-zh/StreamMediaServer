#include "AMFAny.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
#include "mmedia/base/BytesWriter.h"
#include <cstring>
#include <netinet/in.h>

using namespace tmms::mm;
namespace
{
    static std::string empty_string;
}
AMFAny::AMFAny(const std::string &name)
:name_(name)
{

}
AMFAny::AMFAny()
{

}
AMFAny::~AMFAny()
{

}

const std::string &AMFAny::String()
{
    if(this->IsString())
    {
        return this->String();
    }
    RTMP_ERROR << "not a String.";
    return empty_string;
}
bool AMFAny::Boolean()
{
    if(this->IsBoolean())
    {
        return this->Boolean();
    }
    RTMP_ERROR << "not a Boolean.";
    return false;
}
double AMFAny::Number()
{
    if(this->IsNumber())
    {
        return this->Number();
    }
    RTMP_ERROR << "not a Number.";
    return 0.0f;
}
double AMFAny::Date()
{
    if(this->IsDate())
    {
        return this->Date();
    }
    RTMP_ERROR << "not a Date.";
    return 0.0f;
}
AMFObjectPtr AMFAny::Object()
{
    if(this->IsObject())
    {
        return this->Object();
    }
    RTMP_ERROR << "not a Object.";
    return AMFObjectPtr();
}
bool AMFAny::IsString()
{
    return false;
}
bool AMFAny::IsNumber()
{
    return false;
}
bool AMFAny::IsBoolean()
{
    return false;
}
bool AMFAny::IsDate()
{
    return false;
}
bool AMFAny::IsObject()
{
    return false;
}
bool AMFAny::IsNull()
{
    return false;
}

const std::string &AMFAny::Name() const
{
    return name_;
}
int32_t AMFAny::Count() const
{
    return 1;
}

std::string AMFAny::DecodeString(const char *data)
{
    auto len = BytesReader::ReadUint16T(data);
    if(len>0)
    {
        std::string str(data+2,len);
        return str;
    }
    return std::string();
}   

int AMFAny::WriteNumber(char *buf, double value)
{
    uint64_t res;
    uint64_t in;
    memcpy(&in, &value, sizeof(double));

    res = __bswap_64(in);
    memcpy(buf, &res, 8);
    return 8;
}

int32_t AMFAny::EncodeNumber(char *output, double dVal)
{
    char *p = output;

    *p++ = kAMFNumber;

    p += WriteNumber(p,dVal);

    return p - output;
}

int32_t AMFAny::EncodeString(char *output, const std::string& str)
{
    char *p = output;
    auto len = str.size();
    *p++ = kAMFString;

    p += BytesWriter::WriteUint16T(p,len);
    memcpy(p,str.c_str(),len);
    p += len;

    return p - output;
}

int32_t AMFAny::EncodeBoolean(char *output, bool b)
{
    char *p = output;

    *p++ = kAMFBoolean;

    *p++ = b?0x01:0x00;

    return p - output;
}

int AMFAny::EncodeName(char *buf, const std::string &name)
{
    char *old = buf;
    auto len = name.size();
    unsigned short length = htons(len);
    memcpy(buf, &length, 2);
    buf += 2;
    
    memcpy(buf, name.c_str(), len);
    buf += len;
    return len+2;
}

int32_t AMFAny::EncodeNamedNumber(char *output, const std::string &name, double dVal)
{
    char *old = output;

    output += EncodeName(output,name);
    output += EncodeNumber(output,dVal);
    return output - old;
}
int32_t AMFAny::EncodeNamedString(char *output, const std::string &name, const std::string &value)
{
    char *old = output;

    output += EncodeName(output,name);
    output += EncodeString(output,value);
    return output - old;
}

int32_t AMFAny::EncodeNamedBoolean(char *output, const std::string &name, bool bVal)
{
    char *old = output;

    output += EncodeName(output,name);
    output += EncodeBoolean(output,bVal);
    return output - old;
}