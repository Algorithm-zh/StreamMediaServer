#include "TsTool.h"
#include <sstream>

using namespace tmms::mm;


std::string TsTool::HexString(uint32_t s)
{
    std::ostringstream os;
    os << std::hex << s;
    return os.str();
}

static uint32_t __crc32_MPEG_table[256];
static bool __crc32_MPEG_table_initialized = false;
// @see pycrc reflect at https://github.com/winlinvip/pycrc/blob/master/pycrc/algorithms.py#L107
static uint64_t __crc32_reflect(uint64_t data, int width)
{
    uint64_t res = data & 0x01;
    
    for (int i = 0; i < (int)width - 1; i++) {
        data >>= 1;
        res = (res << 1) | (data & 0x01);
    }
    
    return res;
}
    
// @see pycrc gen_table at https://github.com/winlinvip/pycrc/blob/master/pycrc/algorithms.py#L178
static void __crc32_make_table(uint32_t t[256], uint32_t poly, bool reflect_in)
{
    int width = 32; // 32bits checksum.
    uint64_t msb_mask = (uint32_t)(0x01 << (width - 1));
    uint64_t mask = (uint32_t)(((msb_mask - 1) << 1) | 1);
    
    int tbl_idx_width = 8; // table index size.
    int tbl_width = 0x01 << tbl_idx_width; // table size: 256
    
    for (int i = 0; i < (int)tbl_width; i++) {
        uint64_t reg = uint64_t(i);
        
        if (reflect_in) {
            reg = __crc32_reflect(reg, tbl_idx_width);
        }
        
        reg = reg << (width - tbl_idx_width);
        for (int j = 0; j < tbl_idx_width; j++) {
            if ((reg&msb_mask) != 0) {
                reg = (reg << 1) ^ poly;
            } else {
                reg = reg << 1;
            }
        }
        
        if (reflect_in) {
            reg = __crc32_reflect(reg, width);
        }
        
        t[i] = (uint32_t)(reg & mask);
    }
}
 
// @see pycrc table_driven at https://github.com/winlinvip/pycrc/blob/master/pycrc/algorithms.py#L207
static uint32_t __crc32_table_driven(uint32_t* t, const void* buf, int size, uint32_t previous, bool reflect_in, uint32_t xor_in, bool reflect_out, uint32_t xor_out)
{
    int width = 32; // 32bits checksum.
    uint64_t msb_mask = (uint32_t)(0x01 << (width - 1));
    uint64_t mask = (uint32_t)(((msb_mask - 1) << 1) | 1);
    
    int tbl_idx_width = 8; // table index size.
    
    uint8_t* p = (uint8_t*)buf;
    uint64_t reg = 0;
    
    if (!reflect_in) {
        reg = xor_in;
        
        for (int i = 0; i < (int)size; i++) {
            uint8_t tblidx = (uint8_t)((reg >> (width - tbl_idx_width)) ^ p[i]);
            reg = t[tblidx] ^ (reg << tbl_idx_width);
        }
    } else {
        reg = previous ^ __crc32_reflect(xor_in, width);
        
        for (int i = 0; i < (int)size; i++) {
            uint8_t tblidx = (uint8_t)(reg ^ p[i]);
            reg = t[tblidx] ^ (reg >> tbl_idx_width);
        }
        
        reg = __crc32_reflect(reg, width);
    }
    
    if (reflect_out) {
        reg = __crc32_reflect(reg, width);
    }
    
    reg ^= xor_out;
    return (uint32_t)(reg & mask);
}

uint32_t TsTool::CRC32(const void* buf, int size)
{
    // @see golang IEEE of hash/crc32/crc32.go
    // IEEE is by far and away the most common CRC-32 polynomial.
    // Used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
    // @remark The poly of CRC32 IEEE is 0x04C11DB7, its reverse is 0xEDB88320,
    //      please read https://en.wikipedia.org/wiki/Cyclic_redundancy_check
    uint32_t poly = 0x04C11DB7;
    
    bool reflect_in = false;
    uint32_t xor_in = 0xffffffff;
    bool reflect_out = false;
    uint32_t xor_out = 0x0;
    
    if (!__crc32_MPEG_table_initialized) {
        __crc32_make_table(__crc32_MPEG_table, poly, reflect_in);
        __crc32_MPEG_table_initialized = true;
    }
    
    return __crc32_table_driven(__crc32_MPEG_table, buf, size, 0x00, reflect_in, xor_in, reflect_out, xor_out);
}

uint32_t TsTool::CRC32Ieee(const void* buf, int size)  {
// @see golang IEEE of hash/crc32/crc32.go
    // IEEE is by far and away the most common CRC-32 polynomial.
    // Used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
    // @remark The poly of CRC32 IEEE is 0x04C11DB7, its reverse is 0xEDB88320,
    //      please read https://en.wikipedia.org/wiki/Cyclic_redundancy_check
    uint32_t poly = 0x04C11DB7;
    
    bool reflect_in = true;
    uint32_t xor_in = 0xffffffff;
    bool reflect_out = true;
    uint32_t xor_out = 0xffffffff;
    
    if (!__crc32_MPEG_table_initialized) {
        __crc32_make_table(__crc32_MPEG_table, poly, reflect_in);
        __crc32_MPEG_table_initialized = true;
    }
    
    return __crc32_table_driven(__crc32_MPEG_table, buf, size, 0x00, reflect_in, xor_in, reflect_out, xor_out);
}

void TsTool::WritePts(uint8_t *q, int fourbits, int64_t pts)
{	
    //fourbits就是前4位 if pts_dts_flags=='10' fourbits='0010' elseif '11' f='11'
    //如果为11的话还要再存一轮dts
    int val;
    val = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;//pts[32,30]&marker_bit
    *q ++ = val;
    val = (((pts >> 15) & 0x7fff) << 1) | 1;//pts[29,15]&marker_bit
    *q ++ = val >> 8;//val是16位的，先把高8位存储起来
    *q ++ = val;
    val = (((pts) & 0x7fff) << 1) | 1;//pts[14,0]&marker_bit
    *q ++ = val >> 8;;
    *q ++ = val;
};

bool TsTool::IsCodecHeader(const PacketPtr &packet)
{
    if(packet->PacketSize()>1)
    {
        const char *b = packet->Data() + 1;
        if(*b == 0)
        {
            return true;
        }
    }
    return false;
}
 

