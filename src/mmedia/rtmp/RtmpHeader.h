#pragma once
#include <cstdint>
#include <memory>

/*

+--------------+----------------+--------------------+--------------+
| Basic Header | Message Header | Extended Timestamp |  Chunk Data  |
+--------------+----------------+--------------------+--------------+
|                                                    |
|<------------------- Chunk Header ----------------->|
                      Chunk Format

 0               1               2               3
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Timestamp                    |message  length|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  message length (coutinue)    |message type id| msg stream id |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                 msg stream id                 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
                  Message Header
*/

namespace tmms
{
  namespace mm
  {
    enum RtmpMsgType
    {
      kRtmpMsgTypeChunkSize = 1,
      kRtmpMsgTypeBytesRead = 3,
      kRtmpMsgTypeUserControl,
      kRtmpMsgTypeWindowACKSize,
      kRtmpMsgTypeSetPeerBW,
      kRtmpMsgTypeAudio
      = 8,
      kRtmpMsgTypeVideo,
      kRtmpMsgTypeAMF3Meta = 15,
      kRtmpMsgTypeAMF3Shared,
      kRtmpMsgTypeAMF3Message,
      kRtmpMsgTypeAMFMeta ,
      kRtmpMsgTypeAMFShared,
      kRtmpMsgTypeAMFMessage,
      kRtmpMsgTypeMetadata
      = 22,
    };
    //Basic Header中的fmt
    //fmt决定了编码过的消息头的格式。这个字段是一个变长字段，长度取决于cdid
    enum RtmpFmt
    {
      kRtmpFmt0 = 0,
      kRtmpFmt1,
      kRtmpFmt2,
      kRtmpFmt3
    };
    //Chunk Stream ID
    enum RtmpCSID
    {
      kRtmpCSIDCommand = 2,
      kRtmpCSIDAMFIni = 3,
      kRtmpCSIDAudio = 4,
      kRtmpCSIDAMF = 5,
      kRtmpCSIDVideo = 6,
    };
    //Message Stream ID
    #define kRtmpMsID0 0
    #define kRtmpMsID1 1

    #pragma pack(push)
    #pragma pack(1)
    struct RtmpMsgHeader
    {
      uint32_t cs_id{0};
      /* chunk stream id */
      uint32_t timestamp{0}; /* timestamp (delta) */
      uint32_t msg_len{0};
      /* message length */
      uint8_t msg_type{0};
      /* message type id */
      uint32_t msg_sid{0};
      /* message stream id */
      RtmpMsgHeader():cs_id(0),timestamp(0),msg_len(0),msg_type(0),msg_sid(0)
      {}
    };
    #pragma pack()
    using RtmpMsgHeaderPtr = std::shared_ptr<RtmpMsgHeader>;
    }
}
