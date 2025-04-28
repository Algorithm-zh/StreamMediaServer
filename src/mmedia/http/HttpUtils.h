#pragma once
#include "HttpTypes.h"
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>

//工具类
namespace tmms
{
    namespace mm
    {
        class HttpUtils
        {
        public:
            static HttpMethod ParseMethod(const std::string &method);
            static HttpStatusCode ParseStatusCode(int32_t code);
            static std::string ParseStatusMessage(int32_t code);
            static ContentType ParseContentType(const std::string &contentType);
            static const std::string &ContentTypeToString(ContentType contenttype);
            static const std::string &StatusCodeToString(int code);
            static ContentType GetContentType(const std::string &fileName);
            static std::string CharToHex(char c);
            static bool NeedUrlDecoding(const std::string &url);
            static std::string UrlDecode(const std::string &url);      
            static std::string UrlEncode(const std::string &src);

            static std::string& ltrim(std::string &str) 
            {
                //古老写法,//auto p = std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace)));
                //c++11写法
                auto p = std::find_if(str.begin(), str.end(), [](unsigned char ch){return !std::isspace(ch);});
                str.erase(str.begin(), p);
                return str;
            }
            //去除右边的空格
            static std::string& rtrim(std::string &str) 
            {
                auto p = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch){return !std::isspace(ch);});
                str.erase(p.base(), str.end());
                return str;
            }
            //去除左右两端的空格           
            static std::string& Trim(std::string &str) 
            {
                ltrim(rtrim(str));
                return str;
            }     
        };          
    }
}
