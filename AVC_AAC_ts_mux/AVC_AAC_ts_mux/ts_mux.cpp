// AVC_AAC_ts_mux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "fileio.h"
#include "avc_utils.h"
#include "ts_utils.h"
#include "common.h"
#include "ts_mux.h"

unsigned int write_ts_pat_packet(FILE *fd)
{
    unsigned char *ts_pat;
    unsigned int size;
    unsigned int writeret = 0;
    TS_Pat_Packet(&ts_pat, &size);

    writeret = write(fd, ts_pat, size);
    FREEIF(ts_pat);
    return writeret;
}

unsigned int write_ts_pmt_packet(FILE *fd)
{
    unsigned char *ts_pmt;
    unsigned int size;
    unsigned int writeret = 0;
    TS_Pmt_Packet(&ts_pmt, &size);
    writeret = write(fd, ts_pmt, size);
    FREEIF(ts_pmt);
    return writeret;
}

unsigned int readAACframe(FILE *fd, int pos, int filesize, unsigned char **frame, unsigned int *framesize)
{
#define readcountAAC 8000

    unsigned int size = 0;
    unsigned char *startframe = (unsigned char *)malloc(readcountAAC);
    seek(fd, pos, SEEK_SET);
    read(fd, startframe, readcountAAC);
    if (startframe[0] != (unsigned char)0xff || (unsigned char)(startframe[1] & 0xf0) != (unsigned char)0xf0) {
        FREEIF(startframe);
        return 0;
    }
    unsigned char ch3 = startframe[3];
    unsigned char ch4 = startframe[4];
    unsigned char ch5 = startframe[5];
    size = ((ch3 << 11) & 0x1800) | ((ch4 << 3) & 0x07f8) | ((ch5 >> 5) & 0x0007);

    *frame = startframe;
    *framesize = size;
    return size;
}

unsigned int readAVCframe(FILE *fd, int pos, int filesize, unsigned char **frame, unsigned int *framesize)
{
#define readcount (256*1024)

    unsigned int i = 0;
    unsigned char *startframe = (unsigned char *)malloc(readcount);
    seek(fd, pos, SEEK_SET);
    if(pos + readcount > filesize)
    {
        i = filesize - pos;
        read(fd, startframe, i);
    }
    else
    {
        read(fd, startframe, readcount);

        for(i = 4; i < readcount; i++)
        {
            if(startframe[i] == 0x00
                && startframe[i+1] == 0x00
                && startframe[i+2] == 0x00
                && startframe[i+3] == 0x01)
            {
                break;
            }
        }
    }
    *frame = startframe;
    *framesize = i;
    return i;
}

int _tmain(int argc, _TCHAR* argv[])
{
    unsigned int AVCpos = 0;
    unsigned int AACpos = 0;
    int H264filesize = 0;
    int AACfilesize = 0;
    unsigned int audiotimestamp = 0;
    unsigned int videotimestamp = 0;
    ts_adaptation_field_t ts_adaptation_field_Head;
    ts_adaptation_field_t ts_adaptation_field_Tail;
    unsigned int filesize = 0; 
    FILE *H264 = open(AVCFILE, "rb");
    FILE *AAC = open(AACFILE, "rb");
    FILE *OutputTS = open(OUTPUTTS, "wb");
    if (!H264)
    {
        return 0;
    }
    if(!AAC)
    {
        return 0;
    }
    if(!OutputTS)
    {
        return 0;
    }

    seek(H264, 0, SEEK_END);
    H264filesize = ftell(H264);
    seek(H264, 0, SEEK_SET);

    seek(AAC, 0, SEEK_END);
    AACfilesize = ftell(AAC);
    seek(AAC, 0, SEEK_SET);

    printf("=============process start=============\n");
    write_ts_pat_packet(OutputTS);  //创建PAT
    write_ts_pmt_packet(OutputTS);  //创建PMT

    for(;;)
    {
        //interleave audio&video
        if(audiotimestamp > videotimestamp)
        {
            //write video
            unsigned char *AVCframe = NULL; 
            unsigned int AVCframesize = 0;
            ts_pes_t m_video_tspes;
            readAVCframe(H264, AVCpos, H264filesize, &AVCframe, &AVCframesize);
            if(AVCframe == NULL || AVCframesize == 0)
            {
                break;
            }
            AVCpos += AVCframesize;
            NALU_t *nalu = AllocNalu(MAX_ONE_FRAME_SIZE);
            GetAnnexbNALU(AVCframe, AVCframesize, nalu);
            AVC2PES(&m_video_tspes, videotimestamp, AVCframe, AVCframesize);
            if(m_video_tspes.Pes_Packet_Length_Beyond != 0)
            {
                printf("PES_VIDEO : SIZE = %d, videotimestamp = %d \n", m_video_tspes.Pes_Packet_Length_Beyond, videotimestamp);
                //填写自适应段标志
                TS_adaptive_flags_Header(&ts_adaptation_field_Head, audiotimestamp); //填写自适应段标志  ,这里注意 音频类型不要算pcr 所以都用帧尾代替就行
                TS_adaptive_flags_Tail(&ts_adaptation_field_Tail); //填写自适应段标志帧尾
                Pes2Tspacket(OutputTS, &m_video_tspes, TS_H264_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, videotimestamp);
                GetFrametype(nalu);
                if(nalu->Frametype == FRAME_I
                    || nalu->Frametype == FRAME_P
                    || nalu->Frametype == FRAME_B)
                {
                    videotimestamp += 1000*90/30;
                }
                FreeNalu(nalu);
            }
            FREEIF(AVCframe);
        }
        else
        {
            //write audio
            unsigned char *AACframe = NULL; 
            unsigned int AACframesize = 0;
            ts_pes_t m_audio_tspes;
            readAACframe(AAC, AACpos, AACfilesize, &AACframe, &AACframesize);
            if(AACframe == NULL || AACframesize == 0)
            {
                break;
            }
            AACpos += AACframesize;
            AAC2PES(&m_audio_tspes, audiotimestamp, AACframe, AACframesize);
            if(m_audio_tspes.Pes_Packet_Length_Beyond != 0)
            {
                printf("PES_AUDIO : SIZE = %d, audiotimestamp = %d \n", m_audio_tspes.Pes_Packet_Length_Beyond, audiotimestamp);
                //填写自适应段标志
                TS_adaptive_flags_Header(&ts_adaptation_field_Head, audiotimestamp); //填写自适应段标志  ,这里注意 音频类型不要算pcr 所以都用帧尾代替就行
                TS_adaptive_flags_Tail(&ts_adaptation_field_Tail); //填写自适应段标志帧尾
                Pes2Tspacket(OutputTS, &m_audio_tspes, TS_AAC_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, audiotimestamp);
                audiotimestamp += 1024*1000*90/44100;
            }
            FREEIF(AACframe);
        }
    }
    close(H264);
    close(AAC);
    close(OutputTS);
    printf("=============process end=============\n");
    system("pause");
    return 0;
}