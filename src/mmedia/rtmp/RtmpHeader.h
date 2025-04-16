#pragma once 
#include <cstdint>
#include <memory>

namespace tmms
{
    namespace mm
    {
        enum RtmpMsgType 
        {
            kRtmpMsgTypeChunkSize   =  1, 
            kRtmpMsgTypeBytesRead   =  3,  
            kRtmpMsgTypeUserControl,      
            kRtmpMsgTypeWindowACKSize,    
            kRtmpMsgTypeSetPeerBW,        
            kRtmpMsgTypeAudio       =  8,  
            kRtmpMsgTypeVideo,              
            kRtmpMsgTypeAMF3Meta  = 15,  
            kRtmpMsgTypeAMF3Shared,        
            kRtmpMsgTypeAMF3Message,       
            kRtmpMsgTypeAMFMeta ,  
            kRtmpMsgTypeAMFShared,        
            kRtmpMsgTypeAMFMessage,            
            kRtmpMsgTypeMetadata     = 22,  
        };

        enum RtmpFmt
        {
            kRtmpFmt0 = 0,
            kRtmpFmt1,
            kRtmpFmt2,
            kRtmpFmt3
        };
        enum RtmpCSID
        {
            kRtmpCSIDCommand = 2,
            kRtmpCSIDAMFIni = 3,
            kRtmpCSIDAudio = 4,
            kRtmpCSIDAMF = 5,
            kRtmpCSIDVideo = 6,
        };

    #define kRtmpMsID0 0
    #define kRtmpMsID1 1

    #pragma pack(push)
    #pragma pack(1)
        struct RtmpMsgHeader
        {
            uint32_t cs_id{0};       /* chunk stream id */
            uint32_t timestamp{0};  /* timestamp (delta) */
            uint32_t msg_len{0};       /* message length */
            uint8_t  msg_type{0};       /* message type id */
            uint32_t msg_sid{0};       /* message stream id */
            RtmpMsgHeader():cs_id(0),timestamp(0),msg_len(0),msg_type(0),msg_sid(0)
            {}
        };
    #pragma pack()    
        using RtmpMsgHeaderPtr = std::shared_ptr<RtmpMsgHeader>;
    }
}