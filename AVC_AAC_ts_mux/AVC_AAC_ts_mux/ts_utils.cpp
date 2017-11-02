#include "stdafx.h"
#include <stdlib.h>
#include <cstring>
#include "crc.h"
#include "fileio.h"
#include "ts_utils.h"
#include "ts_mux.h"

unsigned int WritePacketNum = 0;
continuity_counter_t continuity_counter;

unsigned int TS_Header(unsigned char *ts_header, unsigned int PID, unsigned char play_init, unsigned char ada_field_control)
{
    ts_header_t tsheader;
    tsheader.sync_byte = TS_SYNC_WORD;
    tsheader.transport_error_indicator = 0x00;
    tsheader.payload_unit_start_indicator = play_init;
    tsheader.transport_priority = 0x00;
    tsheader.pid = PID;
    tsheader.transport_scrambling_control = 0x00;
    tsheader.adaptation_field_control = ada_field_control;

    switch(tsheader.pid)
    {
    case TS_PAT_PID:
        tsheader.continuity_counter = continuity_counter.continuity_counter_pat % 0xF;
        continuity_counter.continuity_counter_pat++;
        break;
    case TS_PMT_PID:
        tsheader.continuity_counter = continuity_counter.continuity_counter_pmt % 0xF;
        continuity_counter.continuity_counter_pmt++;
        break;
    case TS_H264_PID:
        tsheader.continuity_counter = continuity_counter.continuity_counter_video % 0xF;
        continuity_counter.continuity_counter_video++;
        break;
    case TS_AAC_PID:
        tsheader.continuity_counter = continuity_counter.continuity_counter_audio % 0xF;
        continuity_counter.continuity_counter_audio++;
        break;
    default:
        {
            printf("continuity_counter error packet \n");
            return 0;
        }
    }

    ts_header[0] = tsheader.sync_byte;
    ts_header[1] = tsheader.transport_error_indicator << 7
        | tsheader.payload_unit_start_indicator << 6
        | tsheader.transport_priority << 5
        | ((tsheader.pid >> 8) & 0x1f);
    ts_header[2] = tsheader.pid & 0xFF;
    ts_header[3] = tsheader.transport_scrambling_control << 6
        | tsheader.adaptation_field_control << 4
        | tsheader.continuity_counter;

    return TS_HEADER_SIZE;
}

unsigned int TS_Pat(unsigned char *ts_pat)
{
    ts_pat_t tspat;
    unsigned int pat_pos = 0;  
    unsigned int PAT_CRC = 0xFFFFFFFF;

    tspat.table_id = 0x00;
    tspat.section_syntax_indicator = 0x01;
    tspat.zero = 0x00;
    tspat.reserved_1 = 0x03;                                               //设置为11；
    tspat.section_length = 0x0d;                                           //pat结构体长度 16个字节减去上面的3个字节
    tspat.transport_stream_id = 0x01;
    tspat.reserved_2 = 0x03;                                               //设置为11；
    tspat.version_number = 0x00;
    tspat.current_next_indicator = 0x01;                                   //当前的pat 有效
    tspat.section_number = 0x00;
    tspat.last_section_number = 0x00;
    tspat.program_number = 0x01;
    tspat.reserved_3 = 0x07;                                               //设置为111；
    tspat.program_map_PID = TS_PMT_PID;                                    //PMT的PID
    tspat.CRC32 = PAT_CRC;                                                //传输过程中检测的一种算法值 先设定一个填充值

    ts_pat[pat_pos++] = tspat.table_id;
    ts_pat[pat_pos++] = tspat.section_syntax_indicator << 7 | tspat.zero  << 6 | tspat.reserved_1 << 4 | ((tspat.section_length >> 8) & 0x0F);
    ts_pat[pat_pos++] = tspat.section_length & 0x00FF;
    ts_pat[pat_pos++] = tspat.transport_stream_id >> 8;
    ts_pat[pat_pos++] = tspat.transport_stream_id & 0x00FF;
    ts_pat[pat_pos++] = tspat.reserved_2 << 6 | tspat.version_number << 1 | tspat.current_next_indicator;
    ts_pat[pat_pos++] = tspat.section_number;
    ts_pat[pat_pos++] = tspat.last_section_number;
    ts_pat[pat_pos++] = tspat.program_number>>8;
    ts_pat[pat_pos++] = tspat.program_number & 0x00FF;
    ts_pat[pat_pos++]= tspat.reserved_3 << 5 | ((tspat.program_map_PID >> 8) & 0x0F);
    ts_pat[pat_pos++]= tspat.program_map_PID & 0x00FF;
    PAT_CRC = calc_crc32(ts_pat, 12);
    ts_pat[pat_pos++] = (PAT_CRC >> 24) & 0xFF;
    ts_pat[pat_pos++] = (PAT_CRC >> 16) & 0xFF;
    ts_pat[pat_pos++] = (PAT_CRC >> 8) & 0xFF; 
    ts_pat[pat_pos++] = PAT_CRC & 0xFF;

    return pat_pos;
}

unsigned int TS_Pat_Packet(unsigned char **tspatpacket, unsigned int *size)
{
    unsigned int pos = 0;
    unsigned int patsize = 0;
    unsigned char patbuf[TS_PACKET_SIZE] = {0};
    unsigned char *tspat = (unsigned char *)calloc(TS_PACKET_SIZE, sizeof(unsigned char));
    if(tspat == NULL)
    {
        printf("calloc tspat error \n");
        return 0;
    }
    memset(tspat, 0xFF, TS_PACKET_SIZE);
    pos += TS_Header(tspat, TS_PAT_PID, 0x01, 0x01);
    tspat[pos++] = 0; //自适应段的长度为0
    patsize = TS_Pat(patbuf);
    memcpy(&tspat[pos], patbuf, patsize);

    *tspatpacket = tspat;
    *size = TS_PACKET_SIZE;

    return TS_PACKET_SIZE;
}

unsigned int TS_Pmt(unsigned char *ts_pmt)
{
    ts_pmt_t tspmt;
    unsigned int pmt_pos = 0;  
    unsigned int PMT_CRC = 0xFFFFFFFF;

    tspmt.table_id = 0x02;
    tspmt.section_syntax_indicator = 0x01;
    tspmt.zero = 0x00;
    tspmt.reserved_1 = 0x03;
    tspmt.section_length = 0x17;                                           //PMT结构体长度 16 + 5 + 5个字节减去上面的3个字节
    tspmt.program_number = 01;                                             //只有一个节目
    tspmt.reserved_2 = 0x03;
    tspmt.version_number = 0x00;
    tspmt.current_next_indicator = 0x01;                                   //当前的PMT有效
    tspmt.section_number = 0x00;
    tspmt.last_section_number = 0x00;
    tspmt.reserved_3 = 0x07;
    tspmt.PCR_PID = TS_H264_PID ;                                          //视频PID                                   
    tspmt.reserved_4 = 0x0F;
    tspmt.program_info_length = 0x00;                                      //后面无 节目信息描述
    tspmt.stream_type_video = TS_PMT_STREAM_TYPE_VIDEO;                       //视频的类型
    tspmt.reserved_5_video = 0x07;
    tspmt.elementary_PID_video = TS_H264_PID;                              //视频的PID
    tspmt.reserved_6_video= 0x0F;
    tspmt.ES_info_length_video = 0x00;                                     //视频无跟随的相关信息
    tspmt.stream_type_audio = TS_PMT_STREAM_TYPE_AUDIO;                       //音频类型
    tspmt.reserved_5_audio = 0x07;
    tspmt.elementary_PID_audio = TS_AAC_PID;                               //音频PID 
    tspmt.reserved_6_audio = 0x0F;
    tspmt.ES_info_length_audio = 0x00;                                     //音频无跟随的相关信息
    tspmt.CRC32 = PMT_CRC; 

    ts_pmt[pmt_pos++] = tspmt.table_id;
    ts_pmt[pmt_pos++] = tspmt.section_syntax_indicator << 7 | tspmt.zero  << 6 | tspmt.reserved_1 << 4 | ((tspmt.section_length >> 8) & 0x0F);
    ts_pmt[pmt_pos++] = tspmt.section_length & 0x00FF;
    ts_pmt[pmt_pos++] = tspmt.program_number >> 8;
    ts_pmt[pmt_pos++] = tspmt.program_number & 0x00FF;
    ts_pmt[pmt_pos++] = tspmt.reserved_2 << 6 | tspmt.version_number << 1 | tspmt.current_next_indicator;
    ts_pmt[pmt_pos++] = tspmt.section_number;
    ts_pmt[pmt_pos++] = tspmt.last_section_number;
    ts_pmt[pmt_pos++] = tspmt.reserved_3 << 5  | ((tspmt.PCR_PID >> 8) & 0x1F);
    ts_pmt[pmt_pos++] = tspmt.PCR_PID & 0x0FF;
    ts_pmt[pmt_pos++]= tspmt.reserved_4 << 4 | ((tspmt.program_info_length >> 8) & 0x0F);
    ts_pmt[pmt_pos++]= tspmt.program_info_length & 0xFF;
    ts_pmt[pmt_pos++]= tspmt.stream_type_video;                               //视频流的stream_type
    ts_pmt[pmt_pos++]= tspmt.reserved_5_video << 5 | ((tspmt.elementary_PID_video >> 8 ) & 0x1F);
    ts_pmt[pmt_pos++]= tspmt.elementary_PID_video & 0x00FF;
    ts_pmt[pmt_pos++]= tspmt.reserved_6_video<< 4 | ((tspmt.ES_info_length_video >> 8) & 0x0F);
    ts_pmt[pmt_pos++]= tspmt.ES_info_length_video & 0x0FF;
    ts_pmt[pmt_pos++]= tspmt.stream_type_audio;                               //音频流的stream_type
    ts_pmt[pmt_pos++]= tspmt.reserved_5_audio<< 5 | ((tspmt.elementary_PID_audio >> 8 ) & 0x1F);
    ts_pmt[pmt_pos++]= tspmt.elementary_PID_audio & 0x00FF;
    ts_pmt[pmt_pos++]= tspmt.reserved_6_audio << 4 | ((tspmt.ES_info_length_audio >> 8) & 0x0F);
    ts_pmt[pmt_pos++]= tspmt.ES_info_length_audio & 0x0FF;

    PMT_CRC = calc_crc32(ts_pmt, pmt_pos);
    ts_pmt[pmt_pos++] = (PMT_CRC >> 24) & 0xFF;
    ts_pmt[pmt_pos++] = (PMT_CRC >> 16) & 0xFF;
    ts_pmt[pmt_pos++] = (PMT_CRC >> 8) & 0xFF; 
    ts_pmt[pmt_pos++] = PMT_CRC & 0xFF;

    return pmt_pos;

}

unsigned int TS_Pmt_Packet(unsigned char **tspmtpacket, unsigned int *size)
{
    unsigned int pos = 0;
    unsigned int pmtsize = 0;
    unsigned char pmtbuf[TS_PACKET_SIZE] = {0};
    unsigned char *tspmt = (unsigned char *)calloc(TS_PACKET_SIZE, sizeof(unsigned char));
    if(tspmt == NULL)
    {
        printf("calloc ts PMT error");
        return 0;
    }
    memset(tspmt, 0xFF, TS_PACKET_SIZE);
    pos += TS_Header(tspmt, TS_PMT_PID, 0x01, 0x01);
    tspmt[pos++] = 0; //自适应段的长度为0
    pmtsize = TS_Pmt(pmtbuf);
    memcpy(&tspmt[pos], pmtbuf, pmtsize);

    *tspmtpacket = tspmt;
    *size = TS_PACKET_SIZE;

    return TS_PACKET_SIZE;
}

unsigned int AAC2PES(ts_pes_t *AAC_tspes, unsigned int pts, unsigned char *frame, unsigned int framesize)
{
    unsigned int aacpes_pos = 0;
    unsigned int OneFrameLen_AAC = framesize;
    memset(AAC_tspes->Es, 0x00, MAX_ONE_FRAME_SIZE);
    memcpy(AAC_tspes->Es, frame, framesize);

    AAC_tspes->packet_start_code_prefix = 0x000001;
    AAC_tspes->stream_id = TS_AAC_STREAM_ID;
    if (OneFrameLen_AAC > 0xFFFF) //如果一帧数据的大小超出界限
    {
        AAC_tspes->PES_packet_length = 0x00;
        AAC_tspes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;
        aacpes_pos += 16;
    }
    else
    {
        AAC_tspes->PES_packet_length = 0x00;
        AAC_tspes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;
        aacpes_pos += 14;
    }
    AAC_tspes->marker_bit = 0x02;
    AAC_tspes->PES_scrambling_control = 0x00;
    AAC_tspes->PES_priority = 0x00;
    AAC_tspes->data_alignment_indicator = 0x00;
    AAC_tspes->copyright = 0x00;
    AAC_tspes->original_or_copy = 0x00;
    AAC_tspes->PTS_DTS_flags = 0x02;
    AAC_tspes->ESCR_flag = 0x00;
    AAC_tspes->ES_rate_flag = 0x00;
    AAC_tspes->DSM_trick_mode_flag = 0x00;
    AAC_tspes->additional_copy_info_flag = 0x00;
    AAC_tspes->PES_CRC_flag = 0x00;
    AAC_tspes->PES_extension_flag = 0x00;
    AAC_tspes->PES_header_data_length = 0x05;
    //清 0 
    AAC_tspes->tsptsdts.pts_32_30 = 0;
    AAC_tspes->tsptsdts.pts_29_15 = 0;
    AAC_tspes->tsptsdts.pts_14_0 = 0;

    AAC_tspes->tsptsdts.reserved_1 = 0x03; //填写 pts信息
    // Adudiopts大于30bit，使用最高三位 
    if (pts > 0x7FFFFFFF)
    {
        AAC_tspes->tsptsdts.pts_32_30 = (pts >> 30) & 0x07;
        AAC_tspes->tsptsdts.marker_bit1 = 0x01;
    }
    else
    {
        AAC_tspes->tsptsdts.marker_bit1 = 0;
    }
    // Videopts大于15bit，使用更多的位来存储
    if (pts > 0x7FFF)
    {
        AAC_tspes->tsptsdts.pts_29_15 = (pts >> 15) & 0x007FFF;
        AAC_tspes->tsptsdts.marker_bit2 = 0x01;
    }
    else
    {
        AAC_tspes->tsptsdts.marker_bit2 = 0;
    }
    //使用最后15位
    AAC_tspes->tsptsdts.pts_14_0 = pts & 0x007FFF;
    AAC_tspes->tsptsdts.marker_bit3 = 0x01;

    return aacpes_pos;
}

unsigned int AVC2PES(ts_pes_t *AVC_tspes, unsigned int pts, unsigned char *frame, unsigned int framesize)
{
    unsigned int avcpes_pos = 0;
    unsigned int OneFrameLen_H264 = framesize;

    memset(AVC_tspes->Es, 0, MAX_ONE_FRAME_SIZE);
    memcpy(AVC_tspes->Es, frame, framesize);

    AVC_tspes->packet_start_code_prefix = 0x000001;
    AVC_tspes->stream_id = TS_H264_STREAM_ID;                               //E0~EF表示是视频的,C0~DF是音频,H264-- E0
    AVC_tspes->PES_packet_length = 0;                                      //一帧数据的长度 不包含 PES包头 ,这个8 是 自适应的长度,填0 可以自动查找
    AVC_tspes->Pes_Packet_Length_Beyond = OneFrameLen_H264;

    if (OneFrameLen_H264 > 0xFFFF)                                          //如果一帧数据的大小超出界限
    {
        AVC_tspes->PES_packet_length = 0x00;
        AVC_tspes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
        avcpes_pos += 16;
    }
    else
    {
        AVC_tspes->PES_packet_length = 0x00;
        AVC_tspes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
        avcpes_pos += 14;
    }
    AVC_tspes->marker_bit = 0x02;
    AVC_tspes->PES_scrambling_control = 0x00;                               //人选字段 存在，不加扰
    AVC_tspes->PES_priority = 0x00;
    AVC_tspes->data_alignment_indicator = 0x00;
    AVC_tspes->copyright = 0x00;
    AVC_tspes->original_or_copy = 0x00;
    AVC_tspes->PTS_DTS_flags = 0x02;                                         //10'：PTS字段存在,DTS不存在
    AVC_tspes->ESCR_flag = 0x00;
    AVC_tspes->ES_rate_flag = 0x00;
    AVC_tspes->DSM_trick_mode_flag = 0x00;
    AVC_tspes->additional_copy_info_flag = 0x00;
    AVC_tspes->PES_CRC_flag = 0x00;
    AVC_tspes->PES_extension_flag = 0x00;
    AVC_tspes->PES_header_data_length = 0x05;                                //后面的数据 包括了PTS所占的字节数

    //清 0 
    AVC_tspes->tsptsdts.pts_32_30 = 0;
    AVC_tspes->tsptsdts.pts_29_15 = 0;
    AVC_tspes->tsptsdts.pts_14_0 = 0;

    AVC_tspes->tsptsdts.reserved_1 = 0x0003;                                 //填写 pts信息
    // Videopts大于30bit，使用最高三位 
    if (pts > 0x7FFFFFFF)
    {
        AVC_tspes->tsptsdts.pts_32_30 = (pts >> 30) & 0x07;
        AVC_tspes->tsptsdts.marker_bit1 = 0x01;
    }
    else
    {
        AVC_tspes->tsptsdts.marker_bit1 = 0;
    }
    // Videopts大于15bit，使用更多的位来存储
    if (pts > 0x7FFF)
    {
        AVC_tspes->tsptsdts.pts_29_15 = (pts >> 15) & 0x007FFF;
        AVC_tspes->tsptsdts.marker_bit2 = 0x01;
    }
    else
    {
        AVC_tspes->tsptsdts.marker_bit2 = 0;
    }
    //使用最后15位
    AVC_tspes->tsptsdts.pts_14_0 = pts & 0x007FFF;
    AVC_tspes->tsptsdts.marker_bit3 = 0x01;

    return avcpes_pos;
}

unsigned int TS_Adaptive(ts_adaptation_field_t *ts_adaptation_field,unsigned char * buf,unsigned int AdaptiveLength)
{
    unsigned int CurrentAdaptiveLength = 1;                                 //当前已经用的自适应段长度  
    unsigned char Adaptiveflags = 0;                                        //自适应段的标志
    unsigned int adaptive_pos = 0;

    //填写自适应字段
    if (ts_adaptation_field->adaptation_field_length > 0)
    {
        adaptive_pos += 1;                                                  //自适应段的一些标志所占用的1个字节
        CurrentAdaptiveLength += 1;

        if (ts_adaptation_field->discontinuty_indicator)
        {
            Adaptiveflags |= 0x80;
        }
        if (ts_adaptation_field->random_access_indicator)
        {
            Adaptiveflags |= 0x40;
        }
        if (ts_adaptation_field->elementary_stream_priority_indicator)
        {
            Adaptiveflags |= 0x20;
        }
        if (ts_adaptation_field->PCR_flag)
        {
            unsigned long long pcr_base;
            unsigned int pcr_ext;

            pcr_base = (ts_adaptation_field->pcr / 300);
            pcr_ext = (ts_adaptation_field->pcr % 300);

            Adaptiveflags |= 0x10;

            buf[adaptive_pos + 0] = (pcr_base >> 25) & 0xff;
            buf[adaptive_pos + 1] = (pcr_base >> 17) & 0xff;
            buf[adaptive_pos + 2] = (pcr_base >> 9) & 0xff;
            buf[adaptive_pos + 3] = (pcr_base >> 1) & 0xff;
            buf[adaptive_pos + 4] = pcr_base << 7 | pcr_ext >> 8 | 0x7e;
            buf[adaptive_pos + 5] = (pcr_ext) & 0xff;
            adaptive_pos += 6;

            CurrentAdaptiveLength += 6;
        }
        if (ts_adaptation_field->OPCR_flag)
        {
            unsigned long long opcr_base;
            unsigned int opcr_ext;

            opcr_base = (ts_adaptation_field->opcr / 300);
            opcr_ext = (ts_adaptation_field->opcr % 300);

            Adaptiveflags |= 0x08;

            buf[adaptive_pos + 0] = (opcr_base >> 25) & 0xff;
            buf[adaptive_pos + 1] = (opcr_base >> 17) & 0xff;
            buf[adaptive_pos + 2] = (opcr_base >> 9) & 0xff;
            buf[adaptive_pos + 3] = (opcr_base >> 1) & 0xff;
            buf[adaptive_pos + 4] = ((opcr_base << 7) & 0x80) | ((opcr_ext >> 8) & 0x01);
            buf[adaptive_pos + 5] = (opcr_ext) & 0xff;
            adaptive_pos += 6;
            CurrentAdaptiveLength += 6;
        }
        if (ts_adaptation_field->splicing_point_flag)
        {
            buf[adaptive_pos] = ts_adaptation_field->splice_countdown;

            Adaptiveflags |= 0x04;

            adaptive_pos += 1;
            CurrentAdaptiveLength += 1;
        }
        if (ts_adaptation_field->private_data_len > 0)
        {
            Adaptiveflags |= 0x02;
            if ((1 + ts_adaptation_field->private_data_len) > (AdaptiveLength - CurrentAdaptiveLength))
            {
                printf("private_data_len error !\n");
                return getchar();
            }
            else
            {
                buf[adaptive_pos] = ts_adaptation_field->private_data_len;
                adaptive_pos += 1;
                memcpy (buf + adaptive_pos, ts_adaptation_field->private_data, ts_adaptation_field->private_data_len);
                adaptive_pos += ts_adaptation_field->private_data_len;

                CurrentAdaptiveLength += (1 + ts_adaptation_field->private_data_len) ;
            }
        }
        if (ts_adaptation_field->adaptation_field_extension_flag)
        {
            Adaptiveflags |= 0x01;
            buf[adaptive_pos + 1] = 1;
            buf[adaptive_pos + 2] = 0;
            CurrentAdaptiveLength += 2;
        }
        buf[0] = Adaptiveflags;                        //将标志放入内存
    }
    return 1;
}

unsigned int TS_adaptive_flags_Header(ts_adaptation_field_t *ts_adaptation_field,unsigned int pts)
{
    //填写自适应段
    ts_adaptation_field->discontinuty_indicator = 0;
    ts_adaptation_field->random_access_indicator = 0;
    ts_adaptation_field->elementary_stream_priority_indicator = 0;
    ts_adaptation_field->PCR_flag = 1;                                          //只用到这个
    ts_adaptation_field->OPCR_flag = 0;
    ts_adaptation_field->splicing_point_flag = 0;
    ts_adaptation_field->transport_private_data_flag = 0;
    ts_adaptation_field->adaptation_field_extension_flag = 0;

    //需要自己算
    ts_adaptation_field->pcr  = pts * 300;
    ts_adaptation_field->adaptation_field_length = 7;                          //占用7位

    ts_adaptation_field->opcr = 0;
    ts_adaptation_field->splice_countdown = 0;
    ts_adaptation_field->private_data_len = 0;
    return 1;
}

unsigned int TS_adaptive_flags_Tail(ts_adaptation_field_t *ts_adaptation_field)
{
    //填写自适应段
    ts_adaptation_field->discontinuty_indicator = 0;
    ts_adaptation_field->random_access_indicator = 0;
    ts_adaptation_field->elementary_stream_priority_indicator = 0;
    ts_adaptation_field->PCR_flag = 0;                                          //只用到这个
    ts_adaptation_field->OPCR_flag = 0;
    ts_adaptation_field->splicing_point_flag = 0;
    ts_adaptation_field->transport_private_data_flag = 0;
    ts_adaptation_field->adaptation_field_extension_flag = 0;

    //需要自己算
    ts_adaptation_field->pcr  = 0;
    ts_adaptation_field->adaptation_field_length = 1;                          //占用1位标志所用的位

    ts_adaptation_field->opcr = 0;
    ts_adaptation_field->splice_countdown = 0;
    ts_adaptation_field->private_data_len = 0;                    
    return 1;
}

unsigned int Pes2Tspacket(FILE *fd, ts_pes_t *ts_pes, unsigned int stream_pid, ts_adaptation_field_t *ts_adaptation_field_Head,
    ts_adaptation_field_t *ts_adaptation_field_Tail, unsigned int pts)
{
    unsigned int ts_pos = 0;
    unsigned int FirstPacketLoadLength = 0 ;                                   //分片包的第一个包的负载长度
    unsigned int NeafPacketCount = 0;                                          //分片包的个数
    unsigned int AdaptiveLength = 0;                                           //要填写0XFF的长度
    unsigned char *NeafBuf = NULL;                                            //分片包 总负载的指针
    unsigned char TSbuf[TS_PACKET_SIZE];

    memset(TSbuf,0,TS_PACKET_SIZE); 
    FirstPacketLoadLength = 188 - 4 - 1 - ts_adaptation_field_Head->adaptation_field_length - 14; //计算分片包的第一个包的负载长度
    NeafPacketCount += 1;                                                                   //第一个分片包  

    //一个包的情况
    if (ts_pes->Pes_Packet_Length_Beyond < FirstPacketLoadLength)                           //这里是 sps ，pps ，sei等
    {
        memset(TSbuf,0xFF,TS_PACKET_SIZE);
        TS_Header(TSbuf, stream_pid, 0x01, 0x03);                          //PID = TS_H264_PID,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x03,含有调整字段和有效负载 ；
        ts_pos += 4;
        TSbuf[ts_pos + 0] = 184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 1 ;
        TSbuf[ts_pos + 1] = 0x00;
        ts_pos += 2; 
        memset(TSbuf + ts_pos, 0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2));// 9, pes header; 5, dts; 2 [1, ada_field len ;1, 0];
        ts_pos += (184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2);

        TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
        TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
        TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
        TSbuf[ts_pos + 3] = ts_pes->stream_id;
        TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
        TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
        TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
            ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
        TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
            ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
        TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
        ts_pos += 9;

        if (ts_pes->stream_id == TS_H264_STREAM_ID)
        {
            TSbuf[ts_pos + 0] = (((0x3 << 4) | ((pts>> 29) & 0x0E) | 0x01) & 0xff);
            TSbuf[ts_pos + 1]= (((((pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
            TSbuf[ts_pos + 2]= ((((pts >> 14) & 0xfffe) | 0x01) & 0xff);
            TSbuf[ts_pos + 3]= (((((pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
            TSbuf[ts_pos + 4]= ((((pts << 1) & 0xfffe) | 0x01) & 0xff);
            ts_pos += 5;

        }
        else if (ts_pes->stream_id == TS_AAC_STREAM_ID)
        {
            TSbuf[ts_pos + 0] = (((0x3 << 4) | ((pts>> 29) & 0x0E) | 0x01) & 0xff);
            TSbuf[ts_pos + 1]= (((((pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
            TSbuf[ts_pos + 2]= ((((pts >> 14) & 0xfffe) | 0x01) & 0xff);
            TSbuf[ts_pos + 3]= (((((pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
            TSbuf[ts_pos + 4]= ((((pts << 1) & 0xfffe) | 0x01) & 0xff);
            ts_pos += 5;
        }
        else
        {
            printf("ts_pes->stream_id  error 0x%x \n",ts_pes->stream_id);
            return getchar();
        }
        memcpy(TSbuf + ts_pos,ts_pes->Es,ts_pes->Pes_Packet_Length_Beyond);  

        //将包写入文件
        write(fd, TSbuf, TS_PACKET_SIZE);                               //将一包数据写入文件
        WritePacketNum ++;                                                      //已经写入文件的包个数++
        return WritePacketNum;
    }

    NeafPacketCount += (ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)/ 184;     
    NeafPacketCount += 1;                                                                   //最后一个分片包
    AdaptiveLength = 188 - 4 - 1 - ((ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)% 184)  ;  //要填写0XFF的长度
    if ((WritePacketNum % 40) == 0)                                                         //每40个包打一个 pat,一个pmt
    {
        write_ts_pat_packet(fd);                                                         //创建PAT
        write_ts_pmt_packet(fd);                                                         //创建PMT
    }
    //开始处理第一个包,分片包的个数最少也会是两个 
    TS_Header(TSbuf, stream_pid, 0x01, 0x03);                              //PID = TS_H264_PID,有效荷载单元起始指示符_play_init = 0x01, ada_field_C,0x03,含有调整字段和有效负载 ；
    ts_pos += 4;
    TSbuf[ts_pos] = ts_adaptation_field_Head->adaptation_field_length;                      //自适应字段的长度，自己填写的
    ts_pos += 1;                                                       

    TS_Adaptive(ts_adaptation_field_Head,TSbuf + ts_pos,(188 - 4 - 1 - 14));          //填写自适应字段
    ts_pos += ts_adaptation_field_Head->adaptation_field_length;                            //填写自适应段所需要的长度

    TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
    TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
    TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
    TSbuf[ts_pos + 3] = ts_pes->stream_id;
    TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
    TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
    TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
        ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
    TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
        ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
    TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
    ts_pos += 9;

    if (ts_pes->stream_id == TS_H264_STREAM_ID)
    {
        TSbuf[ts_pos + 0] = (((0x3 << 4) | ((pts>> 29) & 0x0E) | 0x01) & 0xff);
        TSbuf[ts_pos + 1]= (((((pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
        TSbuf[ts_pos + 2]= ((((pts >> 14) & 0xfffe) | 0x01) & 0xff);
        TSbuf[ts_pos + 3]= (((((pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
        TSbuf[ts_pos + 4]= ((((pts << 1) & 0xfffe) | 0x01) & 0xff);
        ts_pos += 5;

    }
    else if (ts_pes->stream_id == TS_AAC_STREAM_ID)
    {
        TSbuf[ts_pos + 0] = (((0x3 << 4) | ((pts>> 29) & 0x0E) | 0x01) & 0xff);
        TSbuf[ts_pos + 1]= (((((pts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
        TSbuf[ts_pos + 2]= ((((pts >> 14) & 0xfffe) | 0x01) & 0xff);
        TSbuf[ts_pos + 3]= (((((pts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
        TSbuf[ts_pos + 4]= ((((pts << 1) & 0xfffe) | 0x01) & 0xff);
        ts_pos += 5;
    }
    else
    {
        printf("ts_pes->stream_id  error 0x%x \n",ts_pes->stream_id);
        return getchar();
    }

    NeafBuf = ts_pes->Es ;
    memcpy(TSbuf + ts_pos,NeafBuf,FirstPacketLoadLength);  

    NeafBuf += FirstPacketLoadLength;
    ts_pes->Pes_Packet_Length_Beyond -= FirstPacketLoadLength;
    //将包写入文件
    write(fd, TSbuf, TS_PACKET_SIZE);                               //将一包数据写入文件
    WritePacketNum ++;                                                      //已经写入文件的包个数++

    while(ts_pes->Pes_Packet_Length_Beyond)
    {
        ts_pos = 0;
        memset(TSbuf,0,TS_PACKET_SIZE); 

        if ((WritePacketNum % 40) == 0)                                                         //每40个包打一个 pat,一个pmt
        {
            write_ts_pat_packet(fd);                                                         //创建PAT
            write_ts_pmt_packet(fd);                                                         //创建PMT
        }
        if(ts_pes->Pes_Packet_Length_Beyond >= 184)
        {
            //处理中间包   
            TS_Header(TSbuf,stream_pid,0x00,0x01);     //PID = TS_H264_PID,不是有效荷载单元起始指示符_play_init = 0x00, ada_field_C,0x01,仅有有效负载；    
            ts_pos += 4;
            memcpy(TSbuf + ts_pos,NeafBuf,184); 
            NeafBuf += 184;
            ts_pes->Pes_Packet_Length_Beyond -= 184;
            write(fd, TSbuf, TS_PACKET_SIZE);
        }
        else
        {
            if(ts_pes->Pes_Packet_Length_Beyond == 183||ts_pes->Pes_Packet_Length_Beyond == 182)
            {
                if ((WritePacketNum % 40) == 0)                                                         //每40个包打一个 pat,一个pmt
                {
                    write_ts_pat_packet(fd);                                                         //创建PAT
                    write_ts_pmt_packet(fd);                                                         //创建PMT
                }

                TS_Header(TSbuf, stream_pid, 0x00, 0x03);   //PID = TS_H264_PID,不是有效荷载单元起始指示符_play_init = 0x00, ada_field_C,0x03,含有调整字段和有效负载；
                ts_pos += 4;
                TSbuf[ts_pos + 0] = 0x01;
                TSbuf[ts_pos + 1] = 0x00;
                ts_pos += 2;
                memcpy(TSbuf + ts_pos,NeafBuf,182); 

                NeafBuf += 182;
                ts_pes->Pes_Packet_Length_Beyond -= 182;
                write(fd, TSbuf, TS_PACKET_SIZE);
            }
            else
            {
                if ((WritePacketNum % 40) == 0)                                                         //每40个包打一个 pat,一个pmt
                {
                    write_ts_pat_packet(fd);                                                         //创建PAT
                    write_ts_pmt_packet(fd);                                                         //创建PMT
                }

                TS_Header(TSbuf, stream_pid, 0x00, 0x03);  //PID = TS_H264_PID,不是有效荷载单元起始指示符_play_init = 0x00, ada_field_C,0x03,含有调整字段和有效负载；
                ts_pos += 4;
                TSbuf[ts_pos + 0] = 184-ts_pes->Pes_Packet_Length_Beyond-1 ;
                TSbuf[ts_pos + 1] = 0x00;
                ts_pos += 2;
                memset(TSbuf + ts_pos,0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 2)); 
                ts_pos += (184-ts_pes->Pes_Packet_Length_Beyond-2);
                memcpy(TSbuf + ts_pos,NeafBuf,ts_pes->Pes_Packet_Length_Beyond);
                ts_pes->Pes_Packet_Length_Beyond = 0;
                write(fd, TSbuf, TS_PACKET_SIZE);   //将一包数据写入文件
                WritePacketNum ++;  
            }
        }	
        WritePacketNum ++;  
    }

    return WritePacketNum ;
}