#pragma once

#include <cstdint>
namespace tmms
{
    namespace mm
    {
        //比特流读取工具类 NalBitStream，用于解析 H.264/H.265 NAL 单元中的比特字段
        class NalBitStream 
        {
        public:
            NalBitStream(const char *data, int len);
            //读取1位bit
            uint8_t GetBit();
            //读取多个bit为uint16_t
            uint16_t GetWord(int bits);
            //读取多位bit为uint32_t
            uint32_t GetBitLong(int bits);
            //读取多达64位
            uint64_t GetBit64(int bits);
            //读取一个无符号Exp-Golomb编码的整数
            uint32_t GetUE();
            //读取有符号Exp-Golomb编码整数
            int32_t GetSE();
        private:
            //读取一个字节
            char GetByte();
            const char * data_;
            int len_; //数据长度
            int bits_count_; //当前字节还剩多少bit没读
            int byte_idx_;   //当前读到第几个字节
            char byte_;      //当前字节的值
        };
    }
}
