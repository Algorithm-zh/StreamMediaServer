#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tmms
{
    namespace mm
    {
        enum AMFDataType
        { 
            kAMFNumber = 0, 
            kAMFBoolean, 
            kAMFString, 
            kAMFObject,
            kAMFMovieClip,		/* reserved, not used */
            kAMFNull, 
            kAMFUndefined, 
            kAMFReference, 
            kAMFEcmaArray, 
            kAMFObjectEnd,
            kAMStrictArray, 
            kAMFDate, 
            kAMFLongString, 
            kAMFUnsupported,
            kAMFRecordset,		/* reserved, not used */
            kAMFXMLDoc, 
            kAMFTypedObject,
            kAMFAvmplus,		/* switch to AMF3 */
            kAMFInvalid = 0xff,
        };

        class AMFObject;
        using AMFObjectPtr = std::shared_ptr<AMFObject>;
        class AMFAny:public std::enable_shared_from_this<AMFAny>
        {
        public:
            AMFAny(const std::string &name);
            AMFAny();
            virtual ~AMFAny();

            virtual int Decode(const char *data, int size,bool has=false) = 0;
            virtual const std::string &String();
            virtual bool Boolean();
            virtual double Number();
            virtual double Date();
            virtual AMFObjectPtr Object();

            virtual bool IsString();
            virtual bool IsNumber();
            virtual bool IsBoolean();
            virtual bool IsDate();
            virtual bool IsObject();
            virtual bool IsNull();

            virtual void Dump() const = 0;
            const std::string &Name() const;
            virtual int32_t Count() const;

            static int32_t EncodeNumber(char *output, double dVal);
            static int32_t EncodeString(char *output, const std::string& str);
            static int32_t EncodeBoolean(char *output, bool b);
            static int32_t EncodeNamedNumber(char *output, const std::string &name, double dVal);
            static int32_t EncodeNamedString(char *output, const std::string &name, const std::string &value);
            static int32_t EncodeNamedBoolean(char *output, const std::string &name, bool bVal);
        protected:
            static int EncodeName(char *buf, const std::string &name);
            static int WriteNumber(char *buf, double value);
            static std::string DecodeString(const char *data);
            std::string name_;
        };
    }
}