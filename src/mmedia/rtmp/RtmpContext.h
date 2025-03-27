#pragma once
#include "../../network/net/TcpConnection.h"
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include "RtmpHeader.h"
#include "amf/AMFObject.h"
#include "../base/Packet.h"
#include <functional>
#include <unordered_map>
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    enum RtmpContextState
    {
      kRtmpHandShake = 0,
      kRtmpWaitingDone,
      kRtmpMessage,
    };
    //用户控制消息事件
    enum RtmpEventType
    {
        kRtmpEventTypeStreamBegin = 0,
        kRtmpEventTypeStreamEOF,
        kRtmpEventTypeStreamDry,
        kRtmpEventTypeSetBufferLength,
        kRtmpEventTypeStreamsRecorded,
        kRtmpEventTypePingRequest,
        kRtmpEventTypePingResponse,
    };
    using CommandFunc = std::function<void(AMFObject &obj)>;
    class RtmpContext
    {
    public:
      //握手相关成员函数
      RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client = false);
      ~RtmpContext() = default;
      int32_t Parse(MsgBuffer &buf);
      void OnWriteComplete();
      void StartHandShake();
      //接收成员函数
      int32_t ParseMessage(MsgBuffer &buf);
      void MessageComplete(PacketPtr &&data);
      //发送成员函数
      //核心函数
      bool BuildChunk(const PacketPtr &packet, uint32_t timestamp = 0, bool fmt0 = false);
      void Send();
      bool Ready() const;//当前能不能发送数据
    private:
      //发送相关
      bool BuildChunk(PacketPtr &&packet, uint32_t timestamp = 0, bool fmt0 = false);
      void CheckAndSend();//检测队列里是不是还有等待发送的，并激活业务层查看是不是还有数据要发
      void PushOutQueue(PacketPtr &&packet);

      // 协议控制-send
      void SendSetChunkSize();
      void SendAckWindowSize();
      void SendSetPeerBandwidth();
      void SendBytesRecv();
      void SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2);
      //connect命令消息实现
      //客户端握手成功后会发起connect命令
      void SendConnect();
      void HandleConnect(AMFObject &obj);
      //createStream命令消息实现
      void SendCreateStream();
      void HandleCreateStream(AMFObject &obj);
      //onStatus命令消息实现
      void SendStatus(const std::string &level, const std::string &code, const std::string &description);
      //play命令消息实现
      void SendPlay();
      void HandlePlay(AMFObject &obj);
      void ParseNameAndTcUrl();//解析得到session
      //publish命令消息实现
      void SendPublish();
      void HandlePublish(AMFObject &obj);
      //_result命令消息实现
      void HandleResult(AMFObject &obj);
      //_error命令消息实现
      void HandleError(AMFObject &obj);

      // 协议控制-recv(handle)
      void HandleChunkSize(PacketPtr &packet);
      void HandleAckWindowSize(PacketPtr &packet);
      void HandleUserMessage(PacketPtr &packet);//用户控制消息
      void HandleAmfCommand(PacketPtr &data, bool amf3 = false);

  
      RtmpHandShake handshake_;
      int32_t state_{kRtmpHandShake};
      TcpConnectionPtr connection_;
      RtmpHandler *rtmp_handler_{nullptr};
      std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;
      std::unordered_map<uint32_t, PacketPtr> in_packets_;
      std::unordered_map<uint32_t, uint32_t> in_deltas_;
      std::unordered_map<uint32_t, bool> in_ext_;
      int32_t in_chunk_size_{128};

      char out_buffer_[4096];
      char *out_current_{nullptr};
      std::unordered_map<uint32_t, uint32_t> out_deltas_;
      std::unordered_map<uint32_t, RtmpMsgHeaderPtr> out_message_headers_;
      int32_t out_chunk_size_{4096};
      std::list<PacketPtr> out_waiting_queue_;
      std::list<BufferNodePtr> sending_bufs_;
      std::list<PacketPtr> out_sending_packets_;
      bool sending_{false};
      //peer's ack_size is different of local's ack_size, 
      //but when local's ack_size had send, peer's ack_size can use this buffer
      int32_t ack_size_{2500000};//2.5M
      int32_t in_bytes_{0};
      int32_t last_left_{0};//上次处理完剩余的剩余字节数
      
      std::string app_;//推流点
      std::string tc_url_;
      std::string name_;//流名称
      std::string session_name_;//所有相同的一路推流都归为一个session
      std::string param_;
      bool is_player_{false};//看rtmp是播放的还是推流的
      //保存所有的命令，用名字索引
      std::unordered_map<std::string, CommandFunc> commands_;
      bool is_client_{false};
      
      
    };
    using RtmpContextPtr = std::shared_ptr<RtmpContext>;
  }
}
