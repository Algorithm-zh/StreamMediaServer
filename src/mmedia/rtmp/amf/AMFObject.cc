#include "AMFObject.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
#include "AMFBoolean.h"
#include "AMFNumber.h"
#include "AMFDate.h"
#include "AMFString.h"
#include "AMFLongString.h"
#include "AMFNull.h"

using namespace tmms::mm;

namespace 
{
    static AMFAnyPtr any_ptr_null;
}
AMFObject::AMFObject(const std::string &name)
:AMFAny(name)
{

}
AMFObject::AMFObject()
{

}
AMFObject::~AMFObject()
{

}

int AMFObject::Decode(const char *data, int size,bool has)
{
    std::string nname;
    int32_t parsed = 0;

    while((parsed+3)<=size)
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
            {
                parsed += (nname.size()+2);
                data += (nname.size()+2);  
            }          
        }
        char type = *data++;
        parsed ++;
        switch(type)
        {
            case kAMFNumber:
            {
                std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
                auto len = p->Decode(data,size - parsed);
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
                auto len = p->Decode(data,size - parsed);
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
                auto len = p->Decode(data,size - parsed);
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
                auto len = p->Decode(data,size - parsed,true);
                if(len == -1)
                {
                    return -1;
                }
                data += len;
                parsed += len;
                RTMP_TRACE << "Object ";
                p->Dump();
                properties_.emplace_back(std::move(p));                
                break;                
            }     
            case kAMFNull:
            {
                RTMP_TRACE << "Null.";
                properties_.emplace_back(std::move(std::make_shared<AMFNull>()));
                break;
            }  
            case kAMFEcmaArray:
            {
                int count = BytesReader::ReadUint32T(data);
                parsed += 4;
                data += 4;

                std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                auto len = p->Decode(data,size - parsed,true);
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
                return parsed;
            } 
            case kAMStrictArray:
            {
                int count = BytesReader::ReadUint32T(data);
                parsed += 4;
                data += 4;

                std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                while(count>0)
                {
                    auto len = p->DecodeOnce(data,size - parsed,true);
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
                auto len = p->Decode(data,size - parsed);
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
                auto len = p->Decode(data,size - parsed);
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
            case kAMFMovieClip:
            case kAMFUndefined:
            case kAMFReference:
            case kAMFUnsupported:
            case kAMFRecordset:
            case kAMFXMLDoc:
            case kAMFTypedObject:
            case kAMFAvmplus:
            {
                RTMP_TRACE << " not surpport type:" << type;
                break;
            }
        }
    }
    return parsed;
}
bool AMFObject::IsObject()
{
    return true;
}
AMFObjectPtr AMFObject::Object()
{
    return std::dynamic_pointer_cast<AMFObject>(shared_from_this());
}
void AMFObject::Dump() const
{
    RTMP_TRACE << "Object start";
    for(auto const&p:properties_)
    {
        p->Dump();
    }
    RTMP_TRACE << "Object end";
}

int AMFObject::DecodeOnce(const char *data, int size,bool has)
{
    std::string nname;
    int32_t parsed = 0;


    if(has)
    {
        nname = DecodeString(data);
        if(!nname.empty())
        {
            parsed += (nname.size()+2);
            data += (nname.size()+2);
        }
    }
    char type = *data++;
    parsed ++;
    switch(type)
    {
        case kAMFNumber:
        {
            std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
            auto len = p->Decode(data,size - parsed);
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
            auto len = p->Decode(data,size - parsed);
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
            auto len = p->Decode(data,size - parsed);
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
            auto len = p->Decode(data,size - parsed,true);
            if(len == -1)
            {
                return -1;
            }
            data += len;
            parsed += len;
            RTMP_TRACE << "Object ";
            p->Dump();
            properties_.emplace_back(std::move(p));                
            break;                
        }     
        case kAMFNull:
        {
            RTMP_TRACE << "Null.";
            properties_.emplace_back(std::move(std::make_shared<AMFNull>()));
            break;
        }  
        case kAMFEcmaArray:
        {
            int count = BytesReader::ReadUint32T(data);
            parsed += 4;
            data += 4;

            std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
            auto len = p->Decode(data,size - parsed,true);
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
            return parsed;
        } 
        case kAMStrictArray:
        {
            int count = BytesReader::ReadUint32T(data);
            parsed += 4;
            data += 4;

            std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
            while(count>0)
            {
                auto len = p->DecodeOnce(data,size - parsed,true);
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
            auto len = p->Decode(data,size - parsed);
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
            auto len = p->Decode(data,size - parsed);
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
        case kAMFMovieClip:
        case kAMFUndefined:
        case kAMFReference:
        case kAMFUnsupported:
        case kAMFRecordset:
        case kAMFXMLDoc:
        case kAMFTypedObject:
        case kAMFAvmplus:
        {
            RTMP_TRACE << " not surpport type:" << type;
            break;
        }
    }

    return parsed;
}
const AMFAnyPtr &AMFObject::Property(const std::string &name) const
{
    for(auto const &p:properties_)
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
const AMFAnyPtr &AMFObject::Property(int index) const
{
    if(index<0||index>=properties_.size())
    {
        return any_ptr_null;
    }
    return properties_[index];
}
