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
    unsigned char sync_byte:8;                     //8b ͬ���ֽڣ��̶�Ϊ0x47
    unsigned char transport_error_indicator:1;     //1b �������ָʾ����������tsͷ��adapt�����һ�������ֽڣ�ͨ����Ϊ0������ֽ�����adapt�򳤶���
    unsigned char payload_unit_start_indicator:1;  //1b ���ص�Ԫ��ʼ��ʾ����һ�����������ݰ���ʼʱ���Ϊ1
    unsigned char transport_priority:1;            //1b �������ȼ���0Ϊ�����ȼ���1Ϊ�����ȼ���ͨ��ȡ0
    unsigned int pid:13;                           //13b pidֵ
    unsigned char transport_scrambling_control:2;  //2b ������ſ��ƣ�00��ʾδ����
    unsigned char adaptation_field_control:2;      //2b �Ƿ��������Ӧ������00����������01��Ϊ������Ӧ�򣬽�����Ч���أ���10��Ϊ��������Ӧ������Ч���أ���11��Ϊͬʱ��������Ӧ�����Ч���ء�
    unsigned char continuity_counter:4;            //4b ��������������0-f����ʼֵ��һ��ȡ0����������������
}ts_header_t;

//��Ŀ������
typedef struct _TS_PAT_
{
    unsigned char table_id:8;                   //8b PAT��̶�Ϊ0x00
    unsigned char section_syntax_indicator:1;   //1b �̶�Ϊ1
    unsigned char zero:1;                       //1b �̶�Ϊ0
    unsigned char reserved_1:2;                   //2b �̶�Ϊ11
    unsigned int section_length:12;             //12b �������ݵĳ���
    unsigned int transport_stream_id:16;        //16b ������ID���̶�Ϊ0x0001
    unsigned char reserved_2:2;                   //2b �̶�Ϊ11
    unsigned char version_number:5;             //5b �汾�ţ��̶�Ϊ00000�����PAT�б仯��汾�ż�1
    unsigned char current_next_indicator:1;     //1b �̶�Ϊ1����ʾ���PAT������ã����Ϊ0��Ҫ�ȴ���һ��PAT��
    unsigned char section_number:8;             //8b �̶�Ϊ0x00
    unsigned char last_section_number:8;        //8b �̶�Ϊ0x00
    unsigned int program_number:16;             //16b ��Ŀ��Ϊ0x0000ʱ��ʾ����NIT����Ŀ��Ϊ0x0001ʱ,��ʾ����PMT
    unsigned char reserved_3:3;                   //3b �̶�Ϊ111
    unsigned int network_PID:13;               //13b ��Ŀ�Ŷ�Ӧ���ݵ�PIDֵ ������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID,�����в����� networke_pid 
    unsigned int program_map_PID:13;            //13b ��Ŀ�Ŷ�Ӧ���ݵ�PIDֵ ��Ŀ�Ŵ���0ʱ��Ӧ��PID��ÿ����Ŀ��Ӧһ��
    unsigned int CRC32:32;                      //32b ǰ�����ݵ�CRC32У����
}ts_pat_t;

//��Ŀӳ���
typedef struct _TS_PMT_
{
    unsigned char table_id:8;                     //8b PMT��ȡֵ���⣬0x02
    unsigned char section_syntax_indicator:1;     //1b �̶�Ϊ1
    unsigned char zero:1;                         //1b �̶�Ϊ0
    unsigned char reserved_1:2;                     //2b �̶�Ϊ11
    unsigned int section_length:12;               //12b �������ݵĳ���
    unsigned int program_number:16;               //16b Ƶ�����룬��ʾ��ǰ��PMT��������Ƶ����ȡֵ0x0001
    unsigned char reserved_2:2;                     //2b �̶�Ϊ11
    unsigned char version_number:5;               //5b �汾�ţ��̶�Ϊ00000�����PAT�б仯��汾�ż�1
    unsigned char current_next_indicator:1;       //1b �̶�Ϊ1
    unsigned char section_number:8;               //8b �̶�Ϊ0x00
    unsigned char last_section_number:8;          //8b �̶�Ϊ0x00
    unsigned char reserved_3:3;                     //3b �̶�Ϊ111
    unsigned int PCR_PID:13;                      //13b PCR(��Ŀ�ο�ʱ��)����TS�����PID��ָ��Ϊ��ƵPID
    unsigned char reserved_4:4;                     //4b �̶�Ϊ1111
    unsigned int program_info_length:12;          //12b ��Ŀ������Ϣ��ָ��Ϊ0x000��ʾû��	 
    unsigned char stream_type_video:8;            //8b �����ͣ���־��Video����Audio�����������ݣ�h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03
    unsigned char reserved_5_video:3;             //3b �̶�Ϊ111
    unsigned int elementary_PID_video:13;         //13b ��stream_type��Ӧ��PID
    unsigned char reserved_6_video:4;             //4b �̶�Ϊ1111
    unsigned int ES_info_length_video:12;         //12b ������Ϣ��ָ��Ϊ0x000��ʾû��
    unsigned char stream_type_audio:8;            //8b �����ͣ���־��Video����Audio�����������ݣ�h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03
    unsigned char reserved_5_audio:3;             //3b �̶�Ϊ111
    unsigned int elementary_PID_audio:13;         //13b ��stream_type��Ӧ��PID
    unsigned char reserved_6_audio:4;             //4b �̶�Ϊ1111
    unsigned int ES_info_length_audio:12;         //12b ������Ϣ��ָ��Ϊ0x000��ʾû��
    unsigned int CRC32:32;                        //32b ǰ�����ݵ�CRC32У����
}ts_pmt_t;

//������ ���� 0F ѭ��
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
    unsigned char pts_32_30  : 3;                //��ʾʱ���
    unsigned char marker_bit1: 1;
    unsigned int  pts_29_15 : 15;
    unsigned char marker_bit2 : 1;
    unsigned int  pts_14_0 : 15;
    unsigned char marker_bit3 :1 ;
    unsigned char reserved_2 : 4;
    unsigned char dts_32_30: 3;                  //����ʱ���
    unsigned char marker_bit4 :1;
    unsigned int  dts_29_15 :15;
    unsigned char marker_bit5: 1;
    unsigned int  dts_14_0 :15;
    unsigned char marker_bit6 :1 ;
}ts_ptsdts_t;

//PES���ṹ�壬����PES��ͷ��ES���� ,ͷ 19 ���ֽ�
typedef struct _TS_PES_
{
    unsigned int   packet_start_code_prefix:24;  //��ʼ��0x000001
    unsigned char  stream_id:8;                  //�����������ͺͱ��
    unsigned int   PES_packet_length:16;         //������,����֡���ݵĳ��ȣ�����Ϊ0,Ҫ�Լ���,����16λ�������������Ҫ�Լ���
    unsigned char  marker_bit:2;                 //�����ǣ�'10'
    unsigned char  PES_scrambling_control:2;     //pes����Ч�غɵļ��ŷ�ʽ
    unsigned char  PES_priority:1;               //��Ч���ص����ȼ�
    unsigned char  data_alignment_indicator:1;   //�������Ϊ1����PES����ͷ�����������Ƶ����Ƶsyncword��ʼ�Ĵ��롣
    unsigned char  copyright:1;                  //1:����Ȩ������0������
    unsigned char  original_or_copy:1;           //1;��Ч������ԭʼ�ģ�0����Ч����ʱ������
    unsigned char  PTS_DTS_flags:2;              //'10'��PTS�ֶδ��ڣ���11����PTD��DTS�����ڣ���00������û�У���01�������á�
    unsigned char  ESCR_flag:1;                  //1:escr��׼�ֶ� �� escr��չ�ֶξ����ڣ�0�����κ�escr�ֶδ���
    unsigned char  ES_rate_flag:1;               //1:es_rate�ֶδ��ڣ�0 ��������
    unsigned char  DSM_trick_mode_flag:1;        //1;8�����ؽӷ�ʽ�ֶδ��ڣ�0 ��������
    unsigned char  additional_copy_info_flag:1;  //1:additional_copy_info���ڣ�0: ������
    unsigned char  PES_CRC_flag:1;               //1:crc�ֶδ��ڣ�0��������
    unsigned char  PES_extension_flag:1;         //1����չ�ֶδ��ڣ�0��������
    unsigned char  PES_header_data_length:8;    //�������ݵĳ��ȣ�
    ts_ptsdts_t    tsptsdts;                     //ptsdts�ṹ�����10���ֽ�
    unsigned char  Es[MAX_ONE_FRAME_SIZE];       //һ֡ ԭʼ����
    unsigned int   Pes_Packet_Length_Beyond;     //���PES_packet_length�Ĵ�С��������һ֡���ݵĳ��������������
}ts_pes_t;

//����Ӧ�α�־
typedef struct _TS_Adaptation_field_
{
    unsigned char discontinuty_indicator:1;                //1������ǰ����������Ĳ�����״̬Ϊ��
    unsigned char random_access_indicator:1;               //������һ������ͬPID��PES����Ӧ�ú���PTS�ֶκ�һ��ԭʼ�����ʵ�
    unsigned char elementary_stream_priority_indicator:1;  //���ȼ�
    unsigned char PCR_flag:1;                              //����pcr�ֶ�
    unsigned char OPCR_flag:1;                             //����opcr�ֶ�
    unsigned char splicing_point_flag:1;                   //ƴ�ӵ��־       
    unsigned char transport_private_data_flag:1;           //˽���ֽ�
    unsigned char adaptation_field_extension_flag:1;       //�����ֶ�����չ

    unsigned char adaptation_field_length;                 //����Ӧ�γ���
    unsigned long long  pcr;                               //����Ӧ�����õ��ĵ�pcr
    unsigned long long  opcr;                              //����Ӧ�����õ��ĵ�opcr
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
