#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    //AMF数据类型定义
    enum AMFDataType
    {
        kAMFNumber = 0,
        kAMFBoolean,
        kAMFString,
        kAMFObject,
        kAMFMovieClip,   //reserved, not used
        kAMFNull,
        kAMFUndefined,
        kAMFReference,
        kAMFEcmaArray,
        kAMFObjectEnd,
        kAMFStrictArray,
        kAMFDate,
        kAMFLongString,
        kAMFUnsupported,
        kAMFRecordset,   //reserved, not used
        kAMFXMLDoc,
        kAMFTypedObject,
        kAMFAvmplus,     //switch to AMF3
        kAMFInvalid = 0xff,
    };

    class AMFObject;
    using AMFObjectPtr = std::shared_ptr<AMFObject>;
    class AMFAny : public std::enable_shared_from_this<AMFAny>
    {
    public:
      AMFAny(const std::string &name);
      AMFAny();
      virtual ~AMFAny(); 
      //解析数据成员函数
      //p:需要解析的数据缓存地址，size:需要解析的数据缓存大小，Has:是否带名字
      //返回值：>=0,解析成功的字节数； -1,解析出错
      virtual int Decode(const char *data, int size, bool has = false) = 0;
      //获取数据成员函数
      virtual const std::string &String();
      virtual bool Boolean();
      virtual double Number();
      virtual double Date();
      virtual AMFObjectPtr Object();
      //判断数据类型成员函数
      virtual bool IsString();
      virtual bool IsNumber();
      virtual bool IsBoolean();
      virtual bool IsDate();
      virtual bool IsObject();
      //其它成员函数
      virtual void Dump() const = 0; //输出数据
      const std::string &Name() const;
      virtual int32_t Count() const;//返回有多少个数据
  
      static int32_t EncodeNumber(char *output, double dVal);
      static int WriteNumber(char *buf, double value);
      static int32_t EncodeString(char *output, const std::string& str);
      static int32_t EncodeBoolean(char *output, bool b);
      static int EncodeName(char *buf, const std::string &name);
      static int32_t EncodeNamedNumber(char *output, const std::string &name, double dVal);
      static int32_t EncodeNamedString(char *output, const std::string &name, const std::string &value);
      static int32_t EncodeNamedBoolean(char *output, const std::string &name, bool bVal);

    protected:
      //共同使用的解析函数
      static std::string DecodeString(const char *data);
      std::string name_;
    };
  }
}
