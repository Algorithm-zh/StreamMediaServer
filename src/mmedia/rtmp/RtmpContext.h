#pragma once

#include "network/net/TcpConnection.h"
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include "RtmpHeader.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/amf/AMFObject.h"
#include <cstdint>
#include <unordered_map>

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;

        enum RtmpContextState
        {
            kRtmpHandShake = 0,
            kRtmpWatingDone = 1,
            kRtmpMessage = 2,
        };
        enum RtmpEventType
        {
                    kRtmpEventTypeStreamBegin = 0,
                    kRtmpEventTypeStreamEOF,
                    kRtmpEventTypeStreamDry,
                    kRtmpEventTypeSetBufferLength,
                    kRtmpEventTypeStreamsRecorded,
                    kRtmpEventTypePingRequest ,
                    kRtmpEventTypePingResponse,
        };
        using CommandFunc = std::function<void (AMFObject &obj)>;
        class RtmpContext
        {
        public:
            RtmpContext(const TcpConnectionPtr &conn,RtmpHandler *handler,bool client=false);
            ~RtmpContext() = default;

            int32_t Parse(MsgBuffer &buf);
            void OnWriteComplete();
            void StartHandShake();

            int32_t ParseMessage(MsgBuffer &buf);
            void MessageComplete(PacketPtr && data);
            bool BuildChunk(const PacketPtr &packet,uint32_t timestamp = 0,bool fmt0 = false);
            void Send();
            bool Ready() const;
            void Play(const std::string &url);
            void Publish(const std::string &url);
        private:
            bool BuildChunk (PacketPtr &&packet,uint32_t timestamp = 0,bool fmt0 = false);
            void CheckAndSend();
            void PushOutQueue(PacketPtr && packet);

            void HandleChunkSize(PacketPtr &packet);
            void HandleAckWindowSize(PacketPtr &packet);
            void HandleUserMessage(PacketPtr &packet);
            void HandleAmfCommand(PacketPtr &data,bool amf3=false);

            void SendSetChunkSize();
            void SendAckWindowSize();
            void SendSetPeerBandwidth();
            void SendBytesRecv();
            void SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2);
            void SendConnect();
            void HandleConnect(AMFObject &obj);
            void SendCreateStream();
            void HandleCreateStream(AMFObject &obj);
            void SendStatus(const std::string &level, const std::string &code, const std::string &description);
            
            void SendPlay();
            void HandlePlay(AMFObject &obj);
            void ParseNameAndTcUrl();

            void SendPublish();
            void HandlePublish(AMFObject &obj);

            void HandleResult(AMFObject &obj);
            void HandleError(AMFObject &obj);
            void SetPacketType(PacketPtr &packet);
            RtmpHandShake handshake_;
            int32_t state_ {kRtmpHandShake};
            TcpConnectionPtr connection_;
            RtmpHandler *rtmp_handler_{nullptr};
            std::unordered_map<uint32_t,RtmpMsgHeaderPtr> in_message_headers_;
            std::unordered_map<uint32_t,PacketPtr> in_packets_;
            std::unordered_map<uint32_t,uint32_t> in_deltas_;
            std::unordered_map<uint32_t,bool> in_ext_;
            int32_t in_chunk_size_{128};
            char out_buffer_[4096];
            char *out_current_{nullptr};
            std::unordered_map<uint32_t,uint32_t> out_deltas_;
            std::unordered_map<uint32_t,RtmpMsgHeaderPtr> out_message_headers_;
            int32_t out_chunk_size_{4096};
            std::list<PacketPtr> out_waiting_queue_;
            std::list<BufferNodePtr> sending_bufs_;
            std::list<PacketPtr> out_sending_packets_;
            bool sending_{false};
            int32_t ack_size_{2500000};
            int32_t in_bytes_{0};
            int32_t last_left_{0};
            std::string app_;
            std::string tc_url_;
            std::string name_;
            std::string session_name_;
            std::string param_;
            bool is_player_{false};
            std::unordered_map<std::string,CommandFunc> commands_;
            bool is_client_{false};
        };
        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    }
}