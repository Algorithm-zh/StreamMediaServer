#include "AMFObject.h"
#include "../../base/MMediaLog.h"
#include "../../base/BytesReader.h"
#include "AMFAny.h"
#include "AMFNumber.h"
#include "AMFString.h"
#include "AMFBoolean.h"
#include "AMFDate.h"
#include "AMFLongString.h"
#include "AMFNull.h"
#include "mmedia/rtmp/amf/AMFNull.h"
#include <memory>
#include <utility>
using namespace tmms::mm;
 
namespace
{
  static AMFAnyPtr any_ptr_null;
}
AMFObject::AMFObject(const std::string &name)  
:AMFAny(name)
{
}
 
AMFObject::AMFObject()  {
 
}
 
AMFObject::~AMFObject()  {
 
}
 
int AMFObject::Decode(const char *data, int size, bool has)  {

  std::string nname;
  int32_t parsed = 0;
  while(parsed + 3 < size)
  {
    if(BytesReader::ReadUint24T(data) == 0x000009)
    {
      parsed += 3;
      return parsed;
    }
    if(has)
    {
      nname = DecodeString(data);
      if(!nname.empty())
      {//前两个字节是名字长度
        parsed += (nname.size() + 2);
        data += (nname.size() + 2);
      }
    }
    char type = *data ++;
    parsed ++;
    switch (type) {
      case kAMFNumber:
      {
        std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
        auto len = p->Decode(data, size - parsed);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "Number value:" << p->Number();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFBoolean:
      {
        std::shared_ptr<AMFBoolean> p = std::make_shared<AMFBoolean>(nname);
        auto len = p->Decode(data, size - parsed);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "Boolean value:" << p->Number();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFString:
      {
        std::shared_ptr<AMFString> p = std::make_shared<AMFString>(nname);
        auto len = p->Decode(data, size - parsed);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "String value:" << p->String();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFObject:
      {
        std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
        auto len = p->Decode(data, size - parsed, true);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "Object";
        p->Dump();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFNull:
      {
        std::shared_ptr<AMFNull> p = std::make_shared<AMFNull>(nname);
        RTMP_TRACE << "Null.";
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFEcmaArray:
      {
        //类似unordered_map
        //数组大小
        int count = BytesReader::ReadUint32T(data);
        parsed += 4;
        data += 4;
        std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
        //EcmaArray 需要在 Decode 时检查 kAMFObjectEnd 终止符
        auto len = p->Decode(data, size - parsed, true);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "EcmaArray ";
        p->Dump();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFObjectEnd:
      {
        RTMP_TRACE << "ObjectEnd.";
        //解析完毕
        return parsed;
      }
      case kAMFStrictArray:
      {
        //类似vector
        //数组大小
        int count = BytesReader::ReadUint32T(data);
        parsed += 4;
        data += 4;

        std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
        while(count > 0)
        {
          auto len = p->DecodeOnce(data, size - parsed, true);
          if(len == -1)
          {
            return -1;
          }
          data += len;
          parsed += len;
          count --;
        }
        
        RTMP_TRACE << "EcmaArray ";
        p->Dump();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFDate:
      {
        std::shared_ptr<AMFDate> p = std::make_shared<AMFDate>(nname);
        auto len = p->Decode(data, size - parsed);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "Date value:" << p->Date();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFLongString:
      {
        std::shared_ptr<AMFLongString> p = std::make_shared<AMFLongString>(nname);
        auto len = p->Decode(data, size - parsed);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        RTMP_TRACE << "LongString value:" << p->String();
        properties_.emplace_back(std::move(p));
        break;
      }
      case kAMFUndefined:
      case kAMFReference:
      case kAMFMovieClip:
      case kAMFUnsupported:
      case kAMFXMLDoc:
      case kAMFRecordset:
      case kAMFTypedObject:
      case kAMFAvmplus:
      {
        RTMP_ERROR << "not support type:" << type;
        break;
      }
    }
  }
  return parsed;
}
 
bool AMFObject::IsObject()  {
	return true;
}
 
AMFObjectPtr AMFObject::Object()  {
	return std::dynamic_pointer_cast<AMFObject>(shared_from_this());
}
 
void AMFObject::Dump() const {
  RTMP_TRACE << "Object start";
  for(auto const &p : properties_)
  {
    p->Dump();
  }
}
 
int AMFObject::DecodeOnce(const char *data, int size, bool has)  {

  std::string nname;
  int32_t parsed = 0;
  if(has)
  {
    nname = DecodeString(data);
    if(!nname.empty())
    {//前两个字节是名字长度
      parsed += (nname.size() + 2);
      data += (nname.size() + 2);
    }
  }
  char type = *data ++;
  parsed ++;
  switch (type) {
    case kAMFNumber:
    {
      std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
      auto len = p->Decode(data, size - parsed);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "Number value:" << p->Number();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFBoolean:
    {
      std::shared_ptr<AMFBoolean> p = std::make_shared<AMFBoolean>(nname);
      auto len = p->Decode(data, size - parsed);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "Boolean value:" << p->Number();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFString:
    {
      std::shared_ptr<AMFString> p = std::make_shared<AMFString>(nname);
      auto len = p->Decode(data, size - parsed);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "String value:" << p->String();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFObject:
    {
      std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
      auto len = p->Decode(data, size - parsed, true);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "Object";
      p->Dump();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFNull:
    {
      std::shared_ptr<AMFNull> p = std::make_shared<AMFNull>(nname);
      RTMP_TRACE << "Null.";
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFEcmaArray:
    {
      //类似unordered_map
      //数组大小
      int count = BytesReader::ReadUint32T(data);
      parsed += 4;
      data += 4;
      std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
      //EcmaArray 需要在 Decode 时检查 kAMFObjectEnd 终止符
      auto len = p->Decode(data, size - parsed, true);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "EcmaArray ";
      p->Dump();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFObjectEnd:
    {
      RTMP_TRACE << "ObjectEnd.";
      //解析完毕
      return parsed;
    }
    case kAMFStrictArray:
    {
      //类似vector
      //数组大小
      int count = BytesReader::ReadUint32T(data);
      parsed += 4;
      data += 4;

      std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
      while(count > 0)
      {
        auto len = p->DecodeOnce(data, size - parsed, true);
        if(len == -1)
        {
          return -1;
        }
        data += len;
        parsed += len;
        count --;
      }
      
      RTMP_TRACE << "EcmaArray ";
      p->Dump();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFDate:
    {
      std::shared_ptr<AMFDate> p = std::make_shared<AMFDate>(nname);
      auto len = p->Decode(data, size - parsed);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "Date value:" << p->Date();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFLongString:
    {
      std::shared_ptr<AMFLongString> p = std::make_shared<AMFLongString>(nname);
      auto len = p->Decode(data, size - parsed);
      if(len == -1)
      {
        return -1;
      }
      data += len;
      parsed += len;
      RTMP_TRACE << "LongString value:" << p->String();
      properties_.emplace_back(std::move(p));
      break;
    }
    case kAMFUndefined:
    case kAMFReference:
    case kAMFMovieClip:
    case kAMFUnsupported:
    case kAMFXMLDoc:
    case kAMFRecordset:
    case kAMFTypedObject:
    case kAMFAvmplus:
    {
      RTMP_ERROR << "not support type:" << type;
      break;
    }
  }
  return parsed;
}
 
const AMFAnyPtr &AMFObject::Property(const std::string &name) const {

  for(auto const &p : properties_)
  {
    if(p->Name() == name)
    {
      return p;
    }
    else if(p->IsObject())
    {
      AMFObjectPtr obj = p->Object();
      const AMFAnyPtr &p2 = obj->Property(name);
      if(p2)
      {
        return p2;
      }
    }
  }
  return any_ptr_null;
}
 
const AMFAnyPtr &AMFObject::Property(int index) const {
  if(index < 0 || index >= properties_.size())
  {
    return any_ptr_null;
  }
  return properties_[index];
}
