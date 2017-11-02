#pragma once

#define TS_HEADER_SIZE 4
#define TS_SYNC_WORD 0x47
#define TS_PACKET_SIZE 188
#define TS_PAT_PID 0x00
#define TS_PMT_PID 0xFFF
#define TS_H264_PID 0x100
#define TS_AAC_PID 0x101
#define TS_H264_STREAM_ID 0xE0
#define TS_AAC_STREAM_ID 0xC0

#define TS_PMT_STREAM_TYPE_VIDEO 0x1B
#define TS_PMT_STREAM_TYPE_AUDIO 0x0F

#define MAX_ONE_FRAME_SIZE (256 * 1024)

typedef struct _TS_HEADER_
{
    unsigned char sync_byte:8;                     //8b 同步字节，固定为0x47
    unsigned char transport_error_indicator:1;     //1b 传输错误指示符，表明在ts头的adapt域后由一个无用字节，通常都为0，这个字节算在adapt域长度内
    unsigned char payload_unit_start_indicator:1;  //1b 负载单元起始标示符，一个完整的数据包开始时标记为1
    unsigned char transport_priority:1;            //1b 传输优先级，0为低优先级，1为高优先级，通常取0
    unsigned int pid:13;                           //13b pid值
    unsigned char transport_scrambling_control:2;  //2b 传输加扰控制，00表示未加密
    unsigned char adaptation_field_control:2;      //2b 是否包含自适应区，‘00’保留；‘01’为无自适应域，仅含有效负载；‘10’为仅含自适应域，无有效负载；‘11’为同时带有自适应域和有效负载。
    unsigned char continuity_counter:4;            //4b 递增计数器，从0-f，起始值不一定取0，但必须是连续的
}ts_header_t;

//节目关联表
typedef struct _TS_PAT_
{
    unsigned char table_id:8;                   //8b PAT表固定为0x00
    unsigned char section_syntax_indicator:1;   //1b 固定为1
    unsigned char zero:1;                       //1b 固定为0
    unsigned char reserved_1:2;                   //2b 固定为11
    unsigned int section_length:12;             //12b 后面数据的长度
    unsigned int transport_stream_id:16;        //16b 传输流ID，固定为0x0001
    unsigned char reserved_2:2;                   //2b 固定为11
    unsigned char version_number:5;             //5b 版本号，固定为00000，如果PAT有变化则版本号加1
    unsigned char current_next_indicator:1;     //1b 固定为1，表示这个PAT表可以用，如果为0则要等待下一个PAT表
    unsigned char section_number:8;             //8b 固定为0x00
    unsigned char last_section_number:8;        //8b 固定为0x00
    unsigned int program_number:16;             //16b 节目号为0x0000时表示这是NIT，节目号为0x0001时,表示这是PMT
    unsigned char reserved_3:3;                   //3b 固定为111
    unsigned int network_PID:13;               //13b 节目号对应内容的PID值 网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID,本例中不含有 networke_pid 
    unsigned int program_map_PID:13;            //13b 节目号对应内容的PID值 节目号大于0时对应的PID，每个节目对应一个
    unsigned int CRC32:32;                      //32b 前面数据的CRC32校验码
}ts_pat_t;

//节目映射表
typedef struct _TS_PMT_
{
    unsigned char table_id:8;                     //8b PMT表取值随意，0x02
    unsigned char section_syntax_indicator:1;     //1b 固定为1
    unsigned char zero:1;                         //1b 固定为0
    unsigned char reserved_1:2;                     //2b 固定为11
    unsigned int section_length:12;               //12b 后面数据的长度
    unsigned int program_number:16;               //16b 频道号码，表示当前的PMT关联到的频道，取值0x0001
    unsigned char reserved_2:2;                     //2b 固定为11
    unsigned char version_number:5;               //5b 版本号，固定为00000，如果PAT有变化则版本号加1
    unsigned char current_next_indicator:1;       //1b 固定为1
    unsigned char section_number:8;               //8b 固定为0x00
    unsigned char last_section_number:8;          //8b 固定为0x00
    unsigned char reserved_3:3;                     //3b 固定为111
    unsigned int PCR_PID:13;                      //13b PCR(节目参考时钟)所在TS分组的PID，指定为视频PID
    unsigned char reserved_4:4;                     //4b 固定为1111
    unsigned int program_info_length:12;          //12b 节目描述信息，指定为0x000表示没有	 
    unsigned char stream_type_video:8;            //8b 流类型，标志是Video还是Audio还是其他数据，h.264编码对应0x1b，aac编码对应0x0f，mp3编码对应0x03
    unsigned char reserved_5_video:3;             //3b 固定为111
    unsigned int elementary_PID_video:13;         //13b 与stream_type对应的PID
    unsigned char reserved_6_video:4;             //4b 固定为1111
    unsigned int ES_info_length_video:12;         //12b 描述信息，指定为0x000表示没有
    unsigned char stream_type_audio:8;            //8b 流类型，标志是Video还是Audio还是其他数据，h.264编码对应0x1b，aac编码对应0x0f，mp3编码对应0x03
    unsigned char reserved_5_audio:3;             //3b 固定为111
    unsigned int elementary_PID_audio:13;         //13b 与stream_type对应的PID
    unsigned char reserved_6_audio:4;             //4b 固定为1111
    unsigned int ES_info_length_audio:12;         //12b 描述信息，指定为0x000表示没有
    unsigned int CRC32:32;                        //32b 前面数据的CRC32校验码
}ts_pmt_t;

//计数器 递增 0F 循环
typedef struct _Continuity_Counter
{
    unsigned char continuity_counter_pat;
    unsigned char continuity_counter_pmt;
    unsigned char continuity_counter_video;
    unsigned char continuity_counter_audio;
}continuity_counter_t;

typedef struct _TS_PTSDTS_
{
    unsigned char reserved_1 : 4;
    unsigned char pts_32_30  : 3;                //显示时间戳
    unsigned char marker_bit1: 1;
    unsigned int  pts_29_15 : 15;
    unsigned char marker_bit2 : 1;
    unsigned int  pts_14_0 : 15;
    unsigned char marker_bit3 :1 ;
    unsigned char reserved_2 : 4;
    unsigned char dts_32_30: 3;                  //解码时间戳
    unsigned char marker_bit4 :1;
    unsigned int  dts_29_15 :15;
    unsigned char marker_bit5: 1;
    unsigned int  dts_14_0 :15;
    unsigned char marker_bit6 :1 ;
}ts_ptsdts_t;

//PES包结构体，包括PES包头和ES数据 ,头 19 个字节
typedef struct _TS_PES_
{
    unsigned int   packet_start_code_prefix:24;  //起始：0x000001
    unsigned char  stream_id:8;                  //基本流的类型和编号
    unsigned int   PES_packet_length:16;         //包长度,就是帧数据的长度，可能为0,要自己算,做多16位，如果超出则需要自己算
    unsigned char  marker_bit:2;                 //必须是：'10'
    unsigned char  PES_scrambling_control:2;     //pes包有效载荷的加扰方式
    unsigned char  PES_priority:1;               //有效负载的优先级
    unsigned char  data_alignment_indicator:1;   //如果设置为1表明PES包的头后面紧跟着视频或音频syncword开始的代码。
    unsigned char  copyright:1;                  //1:靠版权保护，0：不靠
    unsigned char  original_or_copy:1;           //1;有效负载是原始的，0：有效负载时拷贝的
    unsigned char  PTS_DTS_flags:2;              //'10'：PTS字段存在，‘11’：PTD和DTS都存在，‘00’：都没有，‘01’：禁用。
    unsigned char  ESCR_flag:1;                  //1:escr基准字段 和 escr扩展字段均存在，0：无任何escr字段存在
    unsigned char  ES_rate_flag:1;               //1:es_rate字段存在，0 ：不存在
    unsigned char  DSM_trick_mode_flag:1;        //1;8比特特接方式字段存在，0 ：不存在
    unsigned char  additional_copy_info_flag:1;  //1:additional_copy_info存在，0: 不存在
    unsigned char  PES_CRC_flag:1;               //1:crc字段存在，0：不存在
    unsigned char  PES_extension_flag:1;         //1：扩展字段存在，0：不存在
    unsigned char  PES_header_data_length:8;    //后面数据的长度，
    ts_ptsdts_t    tsptsdts;                     //ptsdts结构体对象，10个字节
    unsigned char  Es[MAX_ONE_FRAME_SIZE];       //一帧 原始数据
    unsigned int   Pes_Packet_Length_Beyond;     //如果PES_packet_length的大小不能满足一帧数据的长度则用这个代替
}ts_pes_t;

//自适应段标志
typedef struct _TS_Adaptation_field_
{
    unsigned char discontinuty_indicator:1;                //1表明当前传送流分组的不连续状态为真
    unsigned char random_access_indicator:1;               //表明下一个有相同PID的PES分组应该含有PTS字段和一个原始流访问点
    unsigned char elementary_stream_priority_indicator:1;  //优先级
    unsigned char PCR_flag:1;                              //包含pcr字段
    unsigned char OPCR_flag:1;                             //包含opcr字段
    unsigned char splicing_point_flag:1;                   //拼接点标志       
    unsigned char transport_private_data_flag:1;           //私用字节
    unsigned char adaptation_field_extension_flag:1;       //调整字段有扩展

    unsigned char adaptation_field_length;                 //自适应段长度
    unsigned long long  pcr;                               //自适应段中用到的的pcr
    unsigned long long  opcr;                              //自适应段中用到的的opcr
    unsigned char splice_countdown;
    unsigned char private_data_len;
    unsigned char private_data [256];
}ts_adaptation_field_t;

extern unsigned int TS_Header(unsigned char *ts_header, unsigned int PID, unsigned char play_init, unsigned char ada_field_control);

extern unsigned int TS_Pat(unsigned char *ts_pat);

extern unsigned int TS_Pat_Packet(unsigned char **tspatpacket, unsigned int *size);

extern unsigned int TS_Pmt(unsigned char *ts_pmt);

extern unsigned int TS_Pmt_Packet(unsigned char **tspmtpacket, unsigned int *size);

extern unsigned int AAC2PES(ts_pes_t *AAC_tspes, unsigned int pts, unsigned char *frame, unsigned int framesize);

extern unsigned int AVC2PES(ts_pes_t *AVC_tspes, unsigned int pts, unsigned char *frame, unsigned int framesize);

extern unsigned int TS_Adaptive(ts_adaptation_field_t *ts_adaptation_field,unsigned char * buf,unsigned int AdaptiveLength);

extern unsigned int TS_adaptive_flags_Header(ts_adaptation_field_t *ts_adaptation_field,unsigned int pts);

extern unsigned int TS_adaptive_flags_Tail(ts_adaptation_field_t *ts_adaptation_field);

extern unsigned int Pes2Tspacket(FILE *fd, ts_pes_t *ts_pes, unsigned int stream_pid, ts_adaptation_field_t *ts_adaptation_field_Head,
    ts_adaptation_field_t *ts_adaptation_field_Tail, unsigned int pts);
