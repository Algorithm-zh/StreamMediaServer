#pragma once
#include <cstdint>
#include <cstddef>

namespace tmms
{
    namespace mm
    {
        //音频编码格式
        enum AudioCodecID
        {
            kAudioCodecIDLinearPCMPlatformEndian = 0,
            kAudioCodecIDADPCM = 1,
            kAudioCodecIDMP3 = 2,
            kAudioCodecIDLinearPCMLittleEndian = 3,
            kAudioCodecIDNellymoser16kHzMono = 4,
            kAudioCodecIDNellymoser8kHzMono = 5,
            kAudioCodecIDNellymoser = 6,
            kAudioCodecIDReservedG711AlawLogarithmicPCM = 7,
            kAudioCodecIDReservedG711MuLawLogarithmicPCM = 8,
            kAudioCodecIDReserved = 9,
            kAudioCodecIDAAC = 10,
            kAudioCodecIDSpeex = 11,
            kAudioCodecIDOpus = 13,
            kAudioCodecIDReservedMP3_8kHz = 14,
            kAudioCodecIDReservedDeviceSpecificSound = 15,
        };

        //音频采样率
        enum SoundRate
        {
            kSoundRate5512 = 0,
            kSoundRate11025 = 1,
            kSoundRate22050 = 2,
            kSoundRate44100 = 3,
            kSoundRate48000 = 4,
            kSoundRateNB8kHz   = 8,  // NB (narrowband)
            kSoundRateMB12kHz  = 12, // MB (medium-band)
            kSoundRateWB16kHz  = 16, // WB (wideband)
            kSoundRateSWB24kHz = 24, // SWB (super-wideband)
            kSoundRateFB48kHz  = 48, // FB (fullband)
            kSoundRateForbidden = 0xff,        
        };

        //音频采样位数
        enum SoundSize
        {        
            kSoundSizeBits8bit = 0,
            kSoundSizeBits16bit = 1,
            kSoundSizeBitsForbidden = 2,        
        };        

        //音频声道
        enum SoundChannel
        {
            kSoundChannelMono = 0,
            kSoundChannelStereo = 1,
            kSoundChannelForbidden = 2,        
        };         

        //AAC包类型
        enum AACPacketType
        {
            kAACPacketTypeAACSequenceHeader = 0,
            kAACPacketTypeAACRaw = 1,
        };        

    
        enum VideoCodecID
        {
            kVideoCodecIDReserved = 0,
            kVideoCodecIDForbidden = 0,
            kVideoCodecIDReserved1 = 1,        
            kVideoCodecIDSorensonH263 = 2,
            kVideoCodecIDScreenVideo = 3,
            kVideoCodecIDOn2VP6 = 4,
            kVideoCodecIDOn2VP6WithAlphaChannel = 5,
            kVideoCodecIDScreenVideoVersion2 = 6,
            kVideoCodecIDAVC = 7,
            kVideoCodecIDDisabled = 8,
            kVideoCodecIDReserved2 = 9,
            kVideoCodecIDHEVC = 12,
            kVideoCodecIDAV1 = 13,
        };        

        enum AVCPacketType
        {
            kAVCPacketTypeForbidden = 3,
            //avcc格式会用到
            kAVCPacketTypeSequenceHeader = 0,
            kAVCPacketTypeNALU = 1,
            kAVCPacketTypeSequenceHeaderEOF = 2,
        };

        //AAC Sequence Header格式
        enum AACObjectType
        {
            kAACObjectTypeForbidden = 0,
            kAACObjectTypeAacMain = 1,
            kAACObjectTypeAacLC = 2,
            kAACObjectTypeAacSSR = 3,
            kAACObjectTypeAacHE = 5,
            kAACObjectTypeAacHEV2 = 29,
        };
        
        enum NaluType
        {
            kNaluTypeForbidden = 0,

            kNaluTypeNonIDR = 1,
            kNaluTypeDataPartitionA = 2,
            kNaluTypeDataPartitionB = 3,
            kNaluTypeDataPartitionC = 4,
            kNaluTypeIDR = 5,
            kNaluTypeSEI = 6,
            kNaluTypeSPS = 7,
            kNaluTypePPS = 8,
            kNaluTypeAccessUnitDelimiter = 9,
            kNaluTypeEOSequence = 10,
            kNaluTypeEOStream = 11,
            kNaluTypeFilterData = 12,
            kNaluTypeSPSExt = 13,
            kNaluTypePrefixNALU = 14,
            kNaluTypeSubsetSPS = 15,
            kNaluTypeLayerWithoutPartition = 19,
            kNaluTypeCodedSliceExt = 20,
        }; 

        //存储原始音视频数据
        struct SampleBuf
        {
            SampleBuf (const char *buf,size_t s)
            :addr(buf),size(s)
            {}
            const char *addr{nullptr};
            size_t size{0};
        };

        enum TsStreamType
        {
            kTsStreamReserved = 0x00,
            kTsStreamAudioMp3 = 0x04,
            kTsStreamAudioAAC = 0x0f,
            kTsStreamVideoMpeg4 = 0x10,
            kTsStreamAudioMpeg4 = 0x11,
            kTsStreamVideoH264 = 0x1b,
            kTsStreamVideoH265 = 0x24,
            kTsStreamAudioAC3 = 0x81,
            kTsStreamAudioDTS = 0x8a,
            kTsStreamForbidden = 0xff,        
        }; 
        
        enum AacProfile
        {
            AacProfileMain = 0,
            AacProfileLC = 1,
            AacProfileSSR = 2,
            AacProfileReserved = 3,        
        };

    }
}
