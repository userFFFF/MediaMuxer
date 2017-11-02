#ifndef _FLV_MUXER_H_
#define _FLV_MUXER_H_

#define FLV_HEADER_SIZE 9
#define FLV_TAG_HEADER_SIZE 11
#define FLV_PREVIOUS_TAG_SIZE 4
#define FLV_SCRIPT_TAG_SIZE 1024
#define FLV_VIDEO_SEQUENCE_SIZE 1024

#define AVC_SPS_SIZE 64
#define AVC_PPS_SIZE 64
#define AVC_FRAMESIZE_MAX (1024 * 1024)
#define AAC_ADTS_HEADER_SIZE 7

#define ERROR_SUCCESS 0


#define H264FILE "E:\\sourcecode\\visualstudio2010\\Projects\\AVC_STREAM\\FourPeople_720p_2K.h264"
#define AACFILE "E:\\sourcecode\\visualstudio2010\\Projects\\AAC_STREAM\\Audio_aac_2ch_152kbps_44100hz-000603.aac"
#define OUTPUTFLV "E:\\sourcecode\\visualstudio2010\\Projects\\MUX_STREAM\\output.flv"

//#define H264FILE "E:\\sourcecode\\visualstudio2010\\Projects\\FLVMuxer\\x64\\Debug\\FourPeople_720p_2K.h264"
//#define AACFILE "E:\\sourcecode\\visualstudio2010\\Projects\\FLVMuxer\\x64\\Debug\\Audio_aac_2ch_152kbps_44100hz-000603.aac"
//#define OUTPUTFLV "E:\\sourcecode\\visualstudio2010\\Projects\\FLVMuxer\\x64\\Debug\\output.flv"

#define FREEIF(p) do{if(p){free(p); p = NULL;}}while(0);

enum CodecFlvTag
{
    // set to the zero to reserved, for array map.
    CodecFlvTagReserved = 0,

    // 8 = audio
    CodecFlvTagAudio = 8,
    // 9 = video
    CodecFlvTagVideo = 9,
    // 18 = script data
    CodecFlvTagScript = 18,
};

/**
 * The video codec id.
 * @doc video_file_format_spec_v10_1.pdf, page78, E.4.3.1 VIDEODATA
 * CodecID UB [4]
 * Codec Identifier. The following values are defined for FLV:
 *      2 = Sorenson H.263
 *      3 = Screen video
 *      4 = On2 VP6
 *      5 = On2 VP6 with alpha channel
 *      6 = Screen video version 2
 *      7 = AVC
 */
enum VideoCodecId
{
    // set to the zero to reserved, for array map.
    VideoCodecIdReserved = 0,
    VideoCodecIdForbidden = 0,
    VideoCodecIdReserved1 = 1,
    VideoCodecIdReserved2 = 9,

    // for user to disable video, for example, use pure audio hls.
    VideoCodecIdDisabled = 8,

    VideoCodecIdSorensonH263 = 2,
    VideoCodecIdScreenVideo = 3,
    VideoCodecIdOn2VP6 = 4,
    VideoCodecIdOn2VP6WithAlphaChannel = 5,
    VideoCodecIdScreenVideoVersion2 = 6,
    VideoCodecIdAVC = 7,
};

/**
 * Table 7-1 - NAL unit type codes, syntax element categories, and NAL unit type classes
 * ISO_IEC_14496-10-AVC-2012.pdf, page 83.
 */
enum AvcNaluType
{
    // Unspecified
    AvcNaluTypeReserved = 0,
    AvcNaluTypeForbidden = 0,

    // Coded slice of a non-IDR picture slice_layer_without_partitioning_rbsp( )
    AvcNaluTypeNonIDR = 1,
    // Coded slice data partition A slice_data_partition_a_layer_rbsp( )
    AvcNaluTypeDataPartitionA = 2,
    // Coded slice data partition B slice_data_partition_b_layer_rbsp( )
    AvcNaluTypeDataPartitionB = 3,
    // Coded slice data partition C slice_data_partition_c_layer_rbsp( )
    AvcNaluTypeDataPartitionC = 4,
    // Coded slice of an IDR picture slice_layer_without_partitioning_rbsp( )
    AvcNaluTypeIDR = 5,
    // Supplemental enhancement information (SEI) sei_rbsp( )
    AvcNaluTypeSEI = 6,
    // Sequence parameter set seq_parameter_set_rbsp( )
    AvcNaluTypeSPS = 7,
    // Picture parameter set pic_parameter_set_rbsp( )
    AvcNaluTypePPS = 8,
    // Access unit delimiter access_unit_delimiter_rbsp( )
    AvcNaluTypeAccessUnitDelimiter = 9,
    // End of sequence end_of_seq_rbsp( )
    AvcNaluTypeEOSequence = 10,
    // End of stream end_of_stream_rbsp( )
    AvcNaluTypeEOStream = 11,
    // Filler data filler_data_rbsp( )
    AvcNaluTypeFilterData = 12,
    // Sequence parameter set extension seq_parameter_set_extension_rbsp( )
    AvcNaluTypeSPSExt = 13,
    // Prefix NAL unit prefix_nal_unit_rbsp( )
    AvcNaluTypePrefixNALU = 14,
    // Subset sequence parameter set subset_seq_parameter_set_rbsp( )
    AvcNaluTypeSubsetSPS = 15,
    // Coded slice of an auxiliary coded picture without partitioning slice_layer_without_partitioning_rbsp( )
    AvcNaluTypeLayerWithoutPartition = 19,
    // Coded slice extension slice_layer_extension_rbsp( )
    AvcNaluTypeCodedSliceExt = 20,
};

/**
 * The video AVC frame trait(characteristic).
 * @doc video_file_format_spec_v10_1.pdf, page79, E.4.3.2 AVCVIDEOPACKET
 * AVCPacketType IF CodecID == 7 UI8
 * The following values are defined:
 *      0 = AVC sequence header
 *      1 = AVC NALU
 *      2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
 */
enum VideoAvcFrameTrait
{
    // set to the max value to reserved, for array map.
    VideoAvcFrameTraitReserved = 3,
    VideoAvcFrameTraitForbidden = 3,

    VideoAvcFrameTraitSequenceHeader = 0,
    VideoAvcFrameTraitNALU = 1,
    VideoAvcFrameTraitSequenceHeaderEOF = 2,
};

/**
 * The video AVC frame type, such as I/P/B.
 * @doc video_file_format_spec_v10_1.pdf, page78, E.4.3.1 VIDEODATA
 * Frame Type UB [4]
 * Type of video frame. The following values are defined:
 *      1 = key frame (for AVC, a seekable frame)
 *      2 = inter frame (for AVC, a non-seekable frame)
 *      3 = disposable inter frame (H.263 only)
 *      4 = generated key frame (reserved for server use only)
 *      5 = video info/command frame
 */
enum VideoAvcFrameType
{
    // set to the zero to reserved, for array map.
    VideoAvcFrameTypeReserved = 0,
    VideoAvcFrameTypeForbidden = 0,
    VideoAvcFrameTypeReserved1 = 6,

    VideoAvcFrameTypeKeyFrame = 1,
    VideoAvcFrameTypeInterFrame = 2,
    VideoAvcFrameTypeDisposableInterFrame = 3,
    VideoAvcFrameTypeGeneratedKeyFrame = 4,
    VideoAvcFrameTypeVideoInfoFrame = 5,
};

//nal type
enum nal_unit_type_e
{
    NAL_UNKNOWN     = 0,
    NAL_SLICE       = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8
    /* ref_idc == 0 for 6,9,10,11,12 */
};

//frame type
enum _frametype_e
{
    FRAME_I  = 15,
    FRAME_P  = 16,
    FRAME_B  = 17
};

typedef struct _Video_AvcC
{
    unsigned char configurationVersion;  //8；= 0x01
    unsigned char AVCProfileIndication;  //sps即sps的第2字节,所谓的AVCProfileIndication
    unsigned char profile_compatibility; //sps即sps的第3字节,所谓的profile_compatibility
    unsigned char AVCLevelIndication;    //sps即sps的第4字节,所谓的AVCLevelIndication
    unsigned char reserved_1;            //‘111111’b;
    unsigned char lengthSizeMinusOne;    //NALUnitLength 的长度 -1 一般为0x03
    unsigned char reserved_2;            //‘111’b;
    unsigned char numOfSequenceParameterSets;  //一般都是一个
    unsigned int sequenceParameterSetLength;   //sps长度
    unsigned char *sequenceParameterSetNALUnit; //sps数据
    unsigned char numOfPictureParameterSets;   //一般都是一个
    unsigned int  pictureParameterSetLength;   //pps长度
    unsigned char *pictureParameterSetNALUnit;//pps数据
    unsigned char reserved_3;
    unsigned char chroma_format;
    unsigned char reserved_4;
    unsigned char bit_depth_luma_minus8;
    unsigned char reserved_5;
    unsigned char bit_depth_chroma_minus8;
    unsigned char numOfSequenceParameterSetExt;
    unsigned int sequenceParameterSetExtLength;
    unsigned char *sequenceParameterSetExtNALUnit;
}Video_AvcC_t;

typedef struct _NALU_t
{
    unsigned char forbidden_bit;           //! Should always be FALSE
    unsigned char nal_reference_idc;       //! NALU_PRIORITY_xxxx
    unsigned char nal_unit_type;           //! NALU_TYPE_xxxx  
    unsigned int  startcodeprefix_len;     //! 前缀字节数
    unsigned int  len;                     //! 包含nal 头的nal 长度，从第一个00000001到下一个000000001的长度
    unsigned int  max_size;                //! 做多一个nal 的长度
    unsigned char *buf;                   //! 包含nal 头的nal 数据
    unsigned char Frametype;               //! 帧类型
    unsigned int  lost_packets;            //! 预留
}NALU_t;

/**
 * The audio codec id.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 * SoundFormat UB [4]
 * Format of SoundData. The following values are defined:
 *     0 = Linear PCM, platform endian
 *     1 = ADPCM
 *     2 = MP3
 *     3 = Linear PCM, little endian
 *     4 = Nellymoser 16 kHz mono
 *     5 = Nellymoser 8 kHz mono
 *     6 = Nellymoser
 *     7 = G.711 A-law logarithmic PCM
 *     8 = G.711 mu-law logarithmic PCM
 *     9 = reserved
 *     10 = AAC
 *     11 = Speex
 *     14 = MP3 8 kHz
 *     15 = Device-specific sound
 * Formats 7, 8, 14, and 15 are reserved.
 * AAC is supported in Flash Player 9,0,115,0 and higher.
 * Speex is supported in Flash Player 10 and higher.
 */
enum AudioCodecId
{
    // set to the max value to reserved, for array map.
    AudioCodecIdReserved1 = 16,
    AudioCodecIdForbidden = 16,

    // for user to disable audio, for example, use pure video hls.
    AudioCodecIdDisabled = 17,

    AudioCodecIdLinearPCMPlatformEndian = 0,
    AudioCodecIdADPCM = 1,
    AudioCodecIdMP3 = 2,
    AudioCodecIdLinearPCMLittleEndian = 3,
    AudioCodecIdNellymoser16kHzMono = 4,
    AudioCodecIdNellymoser8kHzMono = 5,
    AudioCodecIdNellymoser = 6,
    AudioCodecIdReservedG711AlawLogarithmicPCM = 7,
    AudioCodecIdReservedG711MuLawLogarithmicPCM = 8,
    AudioCodecIdReserved = 9,
    AudioCodecIdAAC = 10,
    AudioCodecIdSpeex = 11,
    AudioCodecIdReservedMP3_8kHz = 14,
    AudioCodecIdReservedDeviceSpecificSound = 15,
};

/**
 * The audio sample rate.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 *      0 = 5.5 kHz = 5512 Hz
 *      1 = 11 kHz = 11025 Hz
 *      2 = 22 kHz = 22050 Hz
 *      3 = 44 kHz = 44100 Hz
 * However, we can extends this table.
 */
enum AudioSampleRate
{
    // set to the max value to reserved, for array map.
    AudioSampleRateReserved = 4,
    AudioSampleRateForbidden = 4,

    AudioSampleRate5512 = 0,
    AudioSampleRate11025 = 1,
    AudioSampleRate22050 = 2,
    AudioSampleRate44100 = 3,
};

/**
 * The audio channel count.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 *      1 = mono
 *      2 = stere0
 * However, we can extends this table.
 */
enum AudioChannelcount
{
    AudioChannelcountmono = 0,
    AudioChannelcountstereo = 1,
};

/**
 * The audio sample size.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 *      1 = mono
 *      2 = stere0
 * However, we can extends this table.
 */
enum Audiosamplesize
{
    Audiosamplesize8bit = 0,
    Audiosamplesize16bit = 1,
};

typedef struct _Audio_Config_
{
    unsigned char audioObjectType;              //5;编解码类型：AAC-LC = 0x02
    unsigned char samplingFrequencyIndex;       //4;采样率 44100 = 0x04
    unsigned char channelConfiguration;         //4;声道 = 2
    unsigned char framelengthFlag;              //1;标志位，位于表明IMDCT窗口长度 = 0
    unsigned char dependsOnCoreCoder;           //1;标志位，表明是否依赖于corecoder = 0
    unsigned char extensionFlag;                //1;选择了AAC-LC = 0
}AudioSpecificConfig_t;

typedef struct _ADTS_HEADER_
{
	unsigned int syncword;  //12 bslbf 同步字The bit string ‘1111 1111 1111’，说明一个ADTS帧的开始
	unsigned int id;        //1 bslbf   MPEG 标示符, 设置为1
	unsigned int layer;     //2 uimsbf Indicates which layer is used. Set to ‘00’
	unsigned int protection_absent;  //1 bslbf  表示是否误码校验
	unsigned int profile;            //2 uimsbf  表示使用哪个级别的AAC，如01 Low Complexity(LC)--- AACLC
	unsigned int sf_index;           //4 uimsbf  表示使用的采样率下标
	unsigned int private_bit;        //1 bslbf 
	unsigned int channel_configuration;  //3 uimsbf  表示声道数
	unsigned int original;               //1 bslbf 
	unsigned int home;                   //1 bslbf 
	/*下面的为改变的参数即每一帧都不同*/
	unsigned int copyright_identification_bit;   //1 bslbf 
	unsigned int copyright_identification_start; //1 bslbf
	unsigned int aac_frame_length;               // 13 bslbf  一个ADTS帧的长度包括ADTS头和raw data block
	unsigned int adts_buffer_fullness;           //11 bslbf     0x7FF 说明是码率可变的码流
	/*no_raw_data_blocks_in_frame 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧.
	所以说number_of_raw_data_blocks_in_frame == 0 
	表示说ADTS帧中有一个AAC数据块并不是说没有。(一个AAC原始帧包含一段时间内1024个采样及相关数据)
    */
	unsigned int no_raw_data_blocks_in_frame;    //2 uimsfb
}adts_header_t;


FILE *open(char *pfilename, char *mode);

int write(FILE *fd, const void *data, int size);

int seek(FILE *fd, int pos, int mod);

int read(FILE *fd, void *data, int size);

int close(FILE *fd);

int flvmuxer_scriptdata(char **scriptdata, int *size, double duration, double filesize,
    bool hasVideo, double videocodecid, double width, double height, double videodatarate, double framerate,
    bool hasAudio, double audiocodecid, double audiosamplerate, double audiosamplesize, bool stereo);

int flv_audiotag(char audiotagheader[FLV_TAG_HEADER_SIZE], int datasize, uint32_t timestamp);

int flv_videotag(char videotagheader[FLV_TAG_HEADER_SIZE], int datasize, uint32_t timestamp);

int flv_sripttag(char sripttagheader[FLV_TAG_HEADER_SIZE], int datasize);

int flvmuxer_scriptpacket(char **flv, int *nb_flv, char *frame, int framesize);

int flvmuxer_audiopacket(char **flv, int *nb_flv, char *frame, int framesize, uint32_t timestamp, uint8_t aac_packet_type);

int flvmuxer_videopacket(char **flv, int *nb_flv, 
    int8_t frame_type, int8_t avc_packet_type, char *frame, int framesize, uint32_t dts, uint32_t pts);

int flvmuxer_audio_specialconfig(char *adtsheader, char **specialconfig, int *size);

int flvmuxer_audio_AudioSpecificConfig_package(char *adtsheader, char **flv, int *nb_flv);

int flvmuxer_video_AVCDecoderConfigurationRecord(char **specialconfig, int *size, char *sps, int sps_size, char *pps, int pps_size);

int flvmuxer_video_AVCDecoderConfigurationRecord_package(char *sps, int sps_size, char *pps, int pps_size, char **flv, int *nb_flv);

int flvmuxer_writeheader(FILE *fd);

int flvmuxer_writesriptdata(FILE *fd, double duration, double filesize);

int flvmuxer_writeaudio_AudioSpecificConfig(FILE *fd, char *adtsheader);

int avc_startcode2(char *buf);

int avc_startcode2(char *buf);

int flvmuv_getAnnexbNALU(NALU_t *nalu);

int flvmuxer_readspspps(FILE *fd, char **sps, int *sps_size, char **pps, int *pps_size);

int flvmuxer_writevideo_AVCDecoderConfigurationRecord(FILE *fd, char *sps, int sps_size, char *pps, int pps_size);

int flvmuxer_writepretagsize(FILE *fd, int pretagsize);

int flvmuxer_writevideodata(FILE *fd, char *frame, int framesize, uint32_t timestamp);

int flvmuxer_writeaudiodata(FILE *fd, char *frame, int framesize, uint32_t timestamp);

int flvmuxer_readAVCframe(FILE *fd, int pos, int filesize, char **frame, int *framesize);

adts_header_t *flvmuxer_parser_adtsheader(char *Adts_Headr_Buf);

AudioSpecificConfig_t flvmuxer_genAudioconfig(char *frame);

int flvmuxer_readAACframe(FILE *fd, int pos, int filesize, char **frame, int *framesize);

#endif