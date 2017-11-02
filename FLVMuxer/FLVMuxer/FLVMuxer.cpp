// FLVMuxer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "bs.h"
#include "amf.h"
#include "FLVMuxer.h"

FILE *open(char *pfilename, char *mode)
{
    return fopen(pfilename, mode);
}

int write(FILE *fd, const void *data, int size)
{
    return fwrite(data, sizeof(char), size, fd);
}

int seek(FILE *fd, int pos, int mod)
{
    return fseek(fd, pos, mod);
}

int read(FILE *fd, void *data, int size)
{
    return fread(data, sizeof(char), size, fd);
}

int close(FILE *fd)
{
    return fclose(fd);
}

int flvmuxer_scriptdata(char **scriptdata, int *size, double duration, double filesize,
    bool hasVideo, double videocodecid, double width, double height, double videodatarate, double framerate,
    bool hasAudio, double audiocodecid, double audiosamplerate, double audiosamplesize, bool stereo)
{
    char *chMetaData = (char *)malloc(FLV_SCRIPT_TAG_SIZE);
    char *chMetaDataTemp = chMetaData;

    /*=============AMF-1=============*/
    // SCRIPTDATASTRING
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_STRING);
    chMetaDataTemp = put_amf_string(chMetaDataTemp, "onMetaData");

    /*=============AMF-2=============*/
    // SCRIPTDATAECMAARRAY
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_ECMA_ARRAY);

    // ECMAArrayLength : UI32 ARRAY length
    chMetaDataTemp = put_be32(chMetaDataTemp, 21);

    // PropertyName
    chMetaDataTemp = put_amf_string(chMetaDataTemp, "MetaDataCreator");
    // Type:UI8 datatype
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_STRING);
    // ScriptDataValue: data
    chMetaDataTemp = put_amf_string(chMetaDataTemp, "created by $$$ @2017");

    // PropertyName
    chMetaDataTemp = put_amf_string(chMetaDataTemp, "hasKeyFrames");
    // Type:UI8 datatype
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    // ScriptDataValue: data
    chMetaDataTemp = put_byte(chMetaDataTemp, 0);

    chMetaDataTemp = put_amf_string(chMetaDataTemp, "hasVideo");
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    chMetaDataTemp = put_byte(chMetaDataTemp, hasVideo);

    chMetaDataTemp = put_amf_string(chMetaDataTemp, "hasAudio");
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    chMetaDataTemp = put_byte(chMetaDataTemp, hasAudio);

    chMetaDataTemp = put_amf_string(chMetaDataTemp, "hasMatadata");
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    chMetaDataTemp = put_byte(chMetaDataTemp, 1);

    chMetaDataTemp = put_amf_string(chMetaDataTemp, "canSeekToEnd");
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    chMetaDataTemp = put_byte(chMetaDataTemp, 0);

    // PropertyName
    chMetaDataTemp = put_amf_string( chMetaDataTemp, "duration");
    // Type:UI8 datatype + ScriptDataValue: data
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)duration);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "width");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)width);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "height");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)height);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "videodatarate");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)videodatarate);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "framerate");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)framerate);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "videocodecid");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)videocodecid);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "audiosamplerate");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)audiosamplerate);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "audiosamplesize");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)audiosamplesize);

    chMetaDataTemp = put_amf_string(chMetaDataTemp, "stereo");
    chMetaDataTemp = put_byte(chMetaDataTemp, AMF_BOOLEAN);
    chMetaDataTemp = put_byte(chMetaDataTemp, stereo);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "audiocodecid");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)audiocodecid);

    chMetaDataTemp = put_amf_string( chMetaDataTemp, "filesize");
    chMetaDataTemp = put_amf_double( chMetaDataTemp, (double)filesize);

    // List Terminator: SCRIPTDATAOBJECTEND
    // SCRIPTDATAOBJECTEND : 0x00 0x00 0x09
    chMetaDataTemp = put_be24( chMetaDataTemp, 9);

    *scriptdata = chMetaData;
    *size = chMetaDataTemp - chMetaData;
    return (chMetaDataTemp - chMetaData);
}

int flv_audiotag(char audiotagheader[FLV_TAG_HEADER_SIZE], int datasize, uint32_t timestamp)
{
    put_byte(audiotagheader, CodecFlvTagAudio);
    put_be24(audiotagheader + 1, datasize);
    put_be24(audiotagheader + 4, timestamp);
    put_byte(audiotagheader + 7, (timestamp >> 24) & 0xFF);
    put_be24(audiotagheader + 8, 0x00);

    return ERROR_SUCCESS;
}

int flv_videotag(char videotagheader[FLV_TAG_HEADER_SIZE], int datasize, uint32_t timestamp)
{
    put_byte(videotagheader, CodecFlvTagVideo);
    put_be24(videotagheader + 1, datasize);
    put_be24(videotagheader + 4, timestamp);
    put_byte(videotagheader + 7, (timestamp >> 24) & 0xFF);
    put_be24(videotagheader + 8, 0x00);

    return ERROR_SUCCESS;
}

int flv_sripttag(char sripttagheader[FLV_TAG_HEADER_SIZE], int datasize)
{
    put_byte(sripttagheader, CodecFlvTagScript);
    put_be24(sripttagheader + 1, datasize);
    put_be24(sripttagheader + 4, 0x00);
    put_byte(sripttagheader + 7, 0x00);
    put_be24(sripttagheader + 8, 0x00);

    return ERROR_SUCCESS;
}

int flvmuxer_scriptpacket(char **flv, int *nb_flv, char *frame, int framesize)
{
    int ret = ERROR_SUCCESS;
    int size = FLV_TAG_HEADER_SIZE + framesize;
    char *data = (char *)malloc(size);
    char *p = data;

    char flvsripttag[FLV_TAG_HEADER_SIZE];
    flv_sripttag(flvsripttag, framesize);
    memcpy(p, flvsripttag, FLV_TAG_HEADER_SIZE);
    p += FLV_TAG_HEADER_SIZE;

    memcpy(p, frame, framesize);
    *flv = data;
    *nb_flv = size;

    return ret;
}

int flvmuxer_audiopacket(char **flv, int *nb_flv, char *frame, int framesize, uint32_t timestamp, uint8_t aac_packet_type)
{
    int ret = ERROR_SUCCESS;
    int size = FLV_TAG_HEADER_SIZE + 2 + framesize;
    char *data = (char *)malloc(size);
    char *p = data;

    char flvauiotag[FLV_TAG_HEADER_SIZE];
    flv_audiotag(flvauiotag, size - FLV_TAG_HEADER_SIZE, timestamp);
    memcpy(p, flvauiotag, FLV_TAG_HEADER_SIZE);
    p += FLV_TAG_HEADER_SIZE;

    uint8_t audio_header = ((AudioCodecIdAAC) << 4) 
        | ((AudioSampleRate44100) << 2) 
        | (Audiosamplesize16bit << 1) 
        | (AudioChannelcountstereo);
    *p++ = audio_header;
    *p++ = aac_packet_type;

    memcpy(p, frame, framesize);
    *flv = data;
    *nb_flv = size;

    return ret;
}

int flvmuxer_videopacket(char **flv, int *nb_flv, 
    int8_t frame_type, int8_t avc_packet_type, char *frame, int framesize, uint32_t dts, uint32_t pts)
{
    int ret = ERROR_SUCCESS;
    int size =  FLV_TAG_HEADER_SIZE + 5 + framesize + 4;//5, AVC format; 4, frame size @buffer header
    char *data = (char *)malloc(size);
    char *p = data;

    char flvvideotag[FLV_TAG_HEADER_SIZE];
    flv_videotag(flvvideotag, size - FLV_TAG_HEADER_SIZE, dts);
    memcpy(p, flvvideotag, FLV_TAG_HEADER_SIZE);
    p += FLV_TAG_HEADER_SIZE;

    *p++ = (frame_type << 4) | VideoCodecIdAVC;

    // AVCPacketType
    *p++ = avc_packet_type;

    // CompositionTime
    // pts = dts + cts, or
    // cts = pts - dts.
    // where cts is the header in rtmp video packet payload header.
    uint32_t cts = pts - dts;
    p = put_be24(p, cts);
    p = put_be32(p, framesize);
    memcpy(p, frame, framesize);
    *flv = data;
    *nb_flv = size;

    return ret;
}

int flvmuxer_audio_specialconfig(char *adtsheader, char **specialconfig, int *size)
{
    AudioSpecificConfig_t adts_config;
    int sequenceheadersize = 2;
    char *sequence = (char *)malloc(sequenceheadersize);
    adts_config = flvmuxer_genAudioconfig(adtsheader);
    sequence[0] = (adts_config.audioObjectType << 3)
        |(adts_config.samplingFrequencyIndex >> 1);
    sequence[1] = (((adts_config.samplingFrequencyIndex) & 0x01) <<  7)
        |(adts_config.channelConfiguration << 3)
        |(adts_config.framelengthFlag << 2)
        |(adts_config.dependsOnCoreCoder << 1)
        |(adts_config.extensionFlag);

    *specialconfig = sequence;
    *size = sequenceheadersize;
    return sequenceheadersize;
}

int flvmuxer_audio_AudioSpecificConfig_package(char *adtsheader, char **flv, int *nb_flv)
{
    char flvauiotag[FLV_TAG_HEADER_SIZE];
    char *AAC_specialconfig = NULL;
    int AAC_specialconfig_size = 0;
    int size = FLV_TAG_HEADER_SIZE + 4;
    char *data = (char *)malloc(size);
    char *p = data;

    flvmuxer_audio_specialconfig(adtsheader, &AAC_specialconfig, &AAC_specialconfig_size);
    flv_audiotag(flvauiotag, size - FLV_TAG_HEADER_SIZE, 0);
    memcpy(p, flvauiotag, FLV_TAG_HEADER_SIZE);
    p += FLV_TAG_HEADER_SIZE;
    uint8_t audio_header = AudioChannelcountstereo & 0x01;
    audio_header |= (Audiosamplesize16bit << 1) & 0x02;
    audio_header |= (AudioSampleRate44100 << 2) & 0x0c;
    audio_header |= (AudioCodecIdAAC << 4) & 0xf0;
    uint8_t AACPackageType = 0;
    *p++ = audio_header;
    *p++ = AACPackageType;
    *p++ = AAC_specialconfig[0];
    *p++ = AAC_specialconfig[1];

    *flv = data;
    *nb_flv = size;
    FREEIF(AAC_specialconfig);

    return size;
}

int flvmuxer_video_AVCDecoderConfigurationRecord(char **specialconfig, int *size, char *sps, int sps_size, char *pps, int pps_size)
{
    unsigned int avcc_pos = 0;
    Video_AvcC_t videoavcc;
    char *data = (char *)malloc(FLV_VIDEO_SEQUENCE_SIZE);
    char *p = data;
    if ((videoavcc.sequenceParameterSetNALUnit = (unsigned char *)calloc(FLV_VIDEO_SEQUENCE_SIZE, sizeof(unsigned char))) == NULL)
    {
        printf("alloc videoavcc.sequenceParameterSetNALUnit error\n");
        return getchar();
    }
    if ((videoavcc.pictureParameterSetNALUnit = (unsigned char *)calloc(FLV_VIDEO_SEQUENCE_SIZE, sizeof(unsigned char))) == NULL)
    {
        printf("alloc videoavcc.pictureParameterSetNALUnit error\n");
        return getchar();
    }

    videoavcc.configurationVersion = 0x01;
    videoavcc.AVCProfileIndication = sps[1];
    videoavcc.profile_compatibility = sps[2];
    videoavcc.AVCLevelIndication = sps[3];
    videoavcc.reserved_1 = 0x3F;
    videoavcc.lengthSizeMinusOne = 0x03;
    videoavcc.reserved_2 = 0x07;
    videoavcc.numOfSequenceParameterSets = 0x01;
    videoavcc.sequenceParameterSetLength = sps_size;
    memcpy(videoavcc.sequenceParameterSetNALUnit,sps,sps_size);
    videoavcc.numOfPictureParameterSets = 0x01;
    videoavcc.pictureParameterSetLength = pps_size;
    memcpy(videoavcc.pictureParameterSetNALUnit,pps,pps_size);
    //下面的按照官方文档可以扩展，结构体 。h文件有定义

    p[0] = videoavcc.configurationVersion;
    p[1] = videoavcc.AVCProfileIndication;
    p[2] = videoavcc.profile_compatibility;
    p[3] = videoavcc.AVCLevelIndication;
    p[4] = ((videoavcc.reserved_1) << 2 ) | videoavcc.lengthSizeMinusOne;
    p[5] = ((videoavcc.reserved_2) << 5)  | videoavcc.numOfSequenceParameterSets;
    p[6] = videoavcc.sequenceParameterSetLength >> 8;
    p[7] = videoavcc.sequenceParameterSetLength & 0xFF;
    avcc_pos += 8;
    memcpy(p + avcc_pos,videoavcc.sequenceParameterSetNALUnit,sps_size);
    avcc_pos += sps_size;
    p[avcc_pos] = videoavcc.numOfPictureParameterSets;
    avcc_pos ++;
    p[avcc_pos] = videoavcc.pictureParameterSetLength >> 8;
    p[avcc_pos + 1] = videoavcc.pictureParameterSetLength & 0xFF;
    avcc_pos += 2;
    memcpy(p + avcc_pos,videoavcc.pictureParameterSetNALUnit,pps_size);
    avcc_pos += pps_size;
    FREEIF(videoavcc.sequenceParameterSetNALUnit);
    FREEIF(videoavcc.pictureParameterSetNALUnit);

    *specialconfig = data;
    *size = avcc_pos;

    return avcc_pos;
}

int flvmuxer_video_AVCDecoderConfigurationRecord_package(char *sps, int sps_size, char *pps, int pps_size, char **flv, int *nb_flv)
{
    char flvvideotag[FLV_TAG_HEADER_SIZE];
    char *AVC_specialconfig = NULL;
    int AVC_specialconfig_size = 0;

    flvmuxer_video_AVCDecoderConfigurationRecord(&AVC_specialconfig, &AVC_specialconfig_size, &sps[4], sps_size-4, &pps[4], pps_size-4);
    int size = FLV_TAG_HEADER_SIZE + AVC_specialconfig_size + 5;
    char *data = (char *)malloc(size);
    char *p = data;
    flv_videotag(flvvideotag, size - FLV_TAG_HEADER_SIZE, 0);
    memcpy(p, flvvideotag, FLV_TAG_HEADER_SIZE);
    p += FLV_TAG_HEADER_SIZE;

    int8_t frame_type = VideoAvcFrameTypeKeyFrame;
    *p++ = (frame_type << 4) | VideoCodecIdAVC;

    // AVCPacketType
    int8_t avc_packet_type = 0;
    *p++ = avc_packet_type;

    uint32_t cts = 0;
    p = put_be24(p, cts);
    memcpy(p, AVC_specialconfig, AVC_specialconfig_size);

    *flv = data;
    *nb_flv = size;
    FREEIF(AVC_specialconfig);

    return size;
}

int flvmuxer_writeheader(FILE *fd)
{
    char flv_header[] = {
        'F', 'L', 'V', // Signatures "FLV"
        (char)0x01, // File version (for example, 0x01 for FLV version 1)
        (char)0x05, // 4, audio; 1, video; 5 audio+video.
        (char)0x00, (char)0x00, (char)0x00, (char)0x09 // DataOffset UI32 The length of this header in bytes
    };

    return write(fd, flv_header, FLV_HEADER_SIZE);
}

int flvmuxer_writesriptdata(FILE *fd, double duration, double filesize)
{
    char *flvsriptpacket = NULL;
    int sriptpacket_size = 0;
    char *script = NULL;
    int scriptsize = 0;
    int writeret = 0;

    flvmuxer_scriptdata(&script, &scriptsize, duration, filesize,
        true, 7.0, 1280.00, 720.00, 2000000, 30.00,
        true, 10.0, 44100.0, 16.0, true);
    flvmuxer_scriptpacket(&flvsriptpacket, &sriptpacket_size, script, scriptsize);
    FREEIF(script);
    writeret = write(fd, flvsriptpacket, sriptpacket_size);
    FREEIF(flvsriptpacket);

    return writeret;
}

int flvmuxer_writeaudio_AudioSpecificConfig(FILE *fd, char *adtsheader)
{
    char *flv = NULL;
    int nb_flv = 0;
    int writeret = 0;

    flvmuxer_audio_AudioSpecificConfig_package(adtsheader, &flv, &nb_flv);
    writeret = write(fd, flv, nb_flv);
    FREEIF(flv);

    return writeret;
}

int avc_startcode2(unsigned char *buf)
{
    if(buf[0] == 0x00
        && buf[1] == 0x00
        && buf[2] == 0x01)
    {
        return 1;
    }

    return 0;
}

int avc_startcode3(unsigned char *buf)
{
    if(buf[0] == 0x00
        && buf[1] == 0x00
        && buf[2] == 0x00
        && buf[3] == 0x01)
    {
        return 1;
    }

    return 0;
}

NALU_t *AllocNalu(int maxsize)
{
    NALU_t *nalu = NULL;
    if((nalu = (NALU_t *)calloc(1, sizeof(NALU_t))) == NULL)
    {
        return NULL;
    }
    nalu->max_size = maxsize;
    if((nalu->buf = (unsigned char *)calloc(nalu->max_size, sizeof(char))) == NULL)
    {
        FREEIF(nalu);
        return NULL;
    }
    return nalu;
}

void FreeNalu(NALU_t *nalu)
{
    FREEIF(nalu->buf);
    FREEIF(nalu);
}

int getframetype(NALU_t *nal)
{
    bs_t s;
    int frame_type = 0; 
    unsigned char * OneFrameBuf_H264 = NULL;
    if ((OneFrameBuf_H264 = (unsigned char *)calloc(nal->len + nal->startcodeprefix_len, sizeof(unsigned char))) == NULL)
    {
        printf("Error malloc OneFrameBuf_H264\n");
        return getchar();
    }
    if (nal->startcodeprefix_len == 3)
    {
        OneFrameBuf_H264[0] = 0x00;
        OneFrameBuf_H264[1] = 0x00;
        OneFrameBuf_H264[2] = 0x01;
        memcpy(OneFrameBuf_H264 + 3,nal->buf,nal->len);
    }
    else if (nal->startcodeprefix_len == 4)
    {
        OneFrameBuf_H264[0] = 0x00;
        OneFrameBuf_H264[1] = 0x00;
        OneFrameBuf_H264[2] = 0x00;
        OneFrameBuf_H264[3] = 0x01;
        memcpy(OneFrameBuf_H264 + 4, nal->buf, nal->len);
    }
    else
    {
        printf("AVC READ ERROR！\n");
    }
    bs_init(&s,OneFrameBuf_H264 + nal->startcodeprefix_len + 1, nal->len - 1);


    if (nal->nal_unit_type == NAL_SLICE || nal->nal_unit_type ==  NAL_SLICE_IDR )
    {
        /* i_first_mb */
        bs_read_ue(&s);
        /* picture type */
        frame_type = bs_read_ue(&s);
        switch(frame_type)
        {
        case 0: case 5: /* P */
            nal->Frametype = FRAME_P;
            break;
        case 1: case 6: /* B */
            nal->Frametype = FRAME_B;
            break;
        case 3: case 8: /* SP */
            nal->Frametype = FRAME_P;
            break;
        case 2: case 7: /* I */
            nal->Frametype = FRAME_I;
            break;
        case 4: case 9: /* SI */
            nal->Frametype = FRAME_I;
            break;
        }
    }
    else if (nal->nal_unit_type == NAL_SEI)
    {
        nal->Frametype = NAL_SEI;
    }
    else if(nal->nal_unit_type == NAL_SPS)
    {
        nal->Frametype = NAL_SPS;
    }
    else if(nal->nal_unit_type == NAL_PPS)
    {
        nal->Frametype = NAL_PPS;
    }

    FREEIF(OneFrameBuf_H264);

    return 1;
}

int flvmuv_getAnnexbNALU(unsigned char *buffer, unsigned int size, NALU_t *nalu)
{    
    int pos = 0;//get next Annexb Nalu position
    unsigned char *frame = buffer;

    //prefix 3 bytes maybe 0x000001
    nalu->startcodeprefix_len = 3;

    //Check whether frame is 0x000001
    if(avc_startcode2(frame) != 1) 
    {
        //If frame is not 0x000001,then read one more byte
        //Check whether frame is 0x00000001
        if (avc_startcode3(frame) != 1)//If not the return -1
        {
            return -1;
        }
        else 
        {
            //If frame is 0x00000001,set the prefix length to 4 bytes
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    } 
    else
    {
        //If frame is 0x000001,set the prefix length to 3 bytes
        pos = 3;
        nalu->startcodeprefix_len = 3;
    }
    //Get next AVC prefix 0x000001 / 0x00000001
    for(; pos < size; pos++)
    {
        if((nalu->startcodeprefix_len == 4 && 1 == avc_startcode3(&frame[pos]))
            || (nalu->startcodeprefix_len == 3 && 1 == avc_startcode2(&frame[pos])))
        {
            break;
        }
        else
        {
            continue;
        }
    }

    nalu->len = pos - nalu->startcodeprefix_len;                      //set nalu len
    memcpy (nalu->buf, &frame[nalu->startcodeprefix_len], nalu->len); //copy nalu buffer to nalu stuct buf
    nalu->forbidden_bit = nalu->buf[0] & 0x80;                        // 1 bit  set nal header bit
    nalu->nal_reference_idc = nalu->buf[0] & 0x60;                    // 2 bit  set idc
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;                      // 5 bit  set uint_type

    getframetype(nalu);
    return nalu->startcodeprefix_len;
}

int flvmuxer_readspspps(FILE *fd, char **sps, int *sps_size, char **pps, int *pps_size)
{
    unsigned char *sps_t = (unsigned char *)malloc(AVC_SPS_SIZE);
    unsigned char *pps_t = (unsigned char *)malloc(AVC_PPS_SIZE);
    int i = 0, j = 0;
    unsigned char spspps[FLV_VIDEO_SEQUENCE_SIZE] = {0};

    read(fd, &spspps,FLV_VIDEO_SEQUENCE_SIZE);

    if(avc_startcode2(spspps) == 1)
    {
        printf("@#$%^*)_+!");
    }
    else if(avc_startcode3(spspps) == 1)
    {
        for (i+=4; ; i++)
        {
            if(spspps[i] == 0x00
                && spspps[i+1] == 0x00
                && spspps[i+2] == 0x00
                && spspps[i+3] == 0x01)
            {
                break;
            }
        }
        j = i;
        memcpy(sps_t, spspps, i);
        for (i+=4; ; i++)
        {
            if(spspps[i] == 0x00
                && spspps[i+1] == 0x00
                && spspps[i+2] == 0x00
                && spspps[i+3] == 0x01)
            {
                break;
            }
        }
        memcpy(pps_t, &spspps[j], i-j);
    }
    AvcNaluType nut = (AvcNaluType)(sps_t[4] & 0x1f);
    AvcNaluType nut1 = (AvcNaluType)(pps_t[4] & 0x1f);

    *sps = (char *)sps_t;
    *pps = (char *)pps_t;
    *sps_size = j;
    *pps_size = i-j;
    return ERROR_SUCCESS;
}

int flvmuxer_writevideo_AVCDecoderConfigurationRecord(FILE *fd, char *sps, int sps_size, char *pps, int pps_size)
{
    char *flv = NULL;
    int nb_flv = 0;
    int writeret = 0;

    flvmuxer_video_AVCDecoderConfigurationRecord_package(sps, sps_size, pps, pps_size, &flv, &nb_flv);
    writeret = write(fd, flv, nb_flv);
    FREEIF(flv);

    return writeret;
}

int flvmuxer_writepretagsize(FILE *fd, int pretagsize)
{
    char flvpts[FLV_PREVIOUS_TAG_SIZE] = {0};
    put_be32(flvpts, pretagsize);

    return write(fd, flvpts, FLV_PREVIOUS_TAG_SIZE);
}

int flvmuxer_writevideodata(FILE *fd, char *frame, int framesize, uint32_t timestamp)
{
    char *flvvideopacket = NULL;
    int videopacket_size = 0;
    int writeret = 0;

    int8_t frame_type = 0;
    int8_t avc_packet_type = 0; 

    AvcNaluType nut = (AvcNaluType)(frame[4] & 0x1f);
    if(nut == AvcNaluTypeSPS 
        || nut == AvcNaluTypePPS)
    {
        frame_type = VideoAvcFrameTypeKeyFrame;
        avc_packet_type = VideoAvcFrameTraitSequenceHeader;
    }
    else if(nut == AvcNaluTypeIDR 
        || nut == AvcNaluTypeNonIDR
        || nut == AvcNaluTypeAccessUnitDelimiter)
    {
        frame_type = VideoAvcFrameTypeInterFrame;
        if (nut == AvcNaluTypeIDR) {
            frame_type = VideoAvcFrameTypeKeyFrame;
        }
        avc_packet_type = VideoAvcFrameTraitNALU;
    }
    flvmuxer_videopacket(&flvvideopacket, &videopacket_size, frame_type, avc_packet_type, &frame[4], framesize-4, timestamp, timestamp);

    writeret = write(fd, flvvideopacket, videopacket_size);
    FREEIF(flvvideopacket);

    return writeret;
}

int flvmuxer_writeaudiodata(FILE *fd, char *frame, int framesize, uint32_t timestamp)
{
    char *flvaudiopacket = NULL;
    int audiopacket_size = 0;
    int writeret = 0;

    flvmuxer_audiopacket(&flvaudiopacket, &audiopacket_size, &frame[7], framesize-7, timestamp, 1);// 0, seqence header; 1, AAC raw data

    writeret = write(fd, flvaudiopacket, audiopacket_size);
    FREEIF(flvaudiopacket);

    return writeret;
}

int flvmuxer_readAVCframe(FILE *fd, int pos, int filesize, char **frame, int *framesize)
{
#define readcount 64000

    int i = 0;
    char *startframe = (char *)malloc(readcount);
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

adts_header_t *flvmuxer_parser_adtsheader(char *Adts_Headr_Buf)
{
    adts_header_t adtsheader = {0};

    adtsheader.syncword = (Adts_Headr_Buf[0] << 4 )  | (Adts_Headr_Buf[1]  >> 4);
    adtsheader.id = ((unsigned int) Adts_Headr_Buf[1] & 0x08) >> 3;
    adtsheader.layer = ((unsigned int) Adts_Headr_Buf[1] & 0x06) >> 1;
    adtsheader.protection_absent = (unsigned int) Adts_Headr_Buf[1] & 0x01;
    adtsheader.profile = ((unsigned int) Adts_Headr_Buf[2] & 0xc0) >> 6;
    adtsheader.sf_index = ((unsigned int) Adts_Headr_Buf[2] & 0x3c) >> 2;
    adtsheader.private_bit = ((unsigned int) Adts_Headr_Buf[2] & 0x02) >> 1;
    adtsheader.channel_configuration = ((((unsigned int) Adts_Headr_Buf[2] & 0x01) << 2) | (((unsigned int) Adts_Headr_Buf[3] & 0xc0) >> 6));
    adtsheader.original = ((unsigned int) Adts_Headr_Buf[3] & 0x20) >> 5;
    adtsheader.home = ((unsigned int) Adts_Headr_Buf[3] & 0x10) >> 4;
    adtsheader.copyright_identification_bit = ((unsigned int) Adts_Headr_Buf[3] & 0x08) >> 3;
    adtsheader.copyright_identification_start = (unsigned int) Adts_Headr_Buf[3] & 0x04 >> 2;		
    adtsheader.aac_frame_length = (((((unsigned int) Adts_Headr_Buf[3]) & 0x03) << 11) | (((unsigned int) Adts_Headr_Buf[4] & 0xFF) << 3)| ((unsigned int) Adts_Headr_Buf[5] & 0xE0) >> 5) ;
    adtsheader.adts_buffer_fullness = (((unsigned int) Adts_Headr_Buf[5] & 0x1f) << 6 | ((unsigned int) Adts_Headr_Buf[6] & 0xfc) >> 2);
    adtsheader.no_raw_data_blocks_in_frame = ((unsigned int) Adts_Headr_Buf[6] & 0x03);

    return &adtsheader;
}


AudioSpecificConfig_t flvmuxer_genAudioconfig(char *frame)
{
    AudioSpecificConfig_t config = {0};
    if(frame == NULL)
    {
        return config;
    }
    config.audioObjectType = (frame[2] & 0xc0) >> 6;
    config.samplingFrequencyIndex = (frame[2] & 0x3c) >> 2;
    config.channelConfiguration = (frame[3] & 0xc0) >> 6;
    config.dependsOnCoreCoder = 0;
    config.extensionFlag = 0;
    config.framelengthFlag = 0;

    return config;
}

int flvmuxer_readAACframe(FILE *fd, int pos, int filesize, char **frame, int *framesize)
{
#define readcountAAC 8000

    int size = 0;
    char *startframe = (char *)malloc(readcountAAC);
    seek(fd, pos, SEEK_SET);
    read(fd, startframe, readcountAAC);
    if (startframe[0] != (char)0xff || (char)(startframe[1] & 0xf0) != (char)0xf0) {
        FREEIF(startframe);
        return 0;
    }
    int8_t ch3 = startframe[3];
    int8_t ch4 = startframe[4];
    int8_t ch5 = startframe[5];
    size = ((ch3 << 11) & 0x1800) | ((ch4 << 3) & 0x07f8) | ((ch5 >> 5) & 0x0007);

    *frame = startframe;
    *framesize = size;
    return size;
}

int _tmain(int argc, _TCHAR* argv[])
{
    FILE *H264 = open(H264FILE, "rb");
    FILE *AAC = open(AACFILE, "rb");
    FILE *OutputFLV = open(OUTPUTFLV, "wb");
    int H264filesize = 0;
    int AACfilesize = 0;
    int pretagsize = 0;
    int AVCpos = 0;
    int AACpos = 0;
    double audiotimestamp = 0;
    double videotimestamp = 0;
    int filesize = 0;
    int i = 0;

    if(OutputFLV == NULL)
    {
        printf("OUTPUT FLV OPEN ERROR \n");
        return 0;
    }
    if(H264 == NULL)
    {
        printf("H264 FILE OPEN ERROR \n");
        return 0;
    }
    if(AAC == NULL)
    {
        printf("AAC FILE OPEN ERROR \n");
        return 0;
    }
    printf("=============process start=============\n");
    seek(H264, 0, SEEK_END);
    H264filesize = ftell(H264);
    seek(H264, 0, SEEK_SET);

    seek(AAC, 0, SEEK_END);
    AACfilesize = ftell(AAC);
    seek(AAC, 0, SEEK_SET);

    //write header
    filesize += flvmuxer_writeheader(OutputFLV);

    //write previous tag size
    filesize += flvmuxer_writepretagsize(OutputFLV, 0);

    //write script data
    pretagsize = flvmuxer_writesriptdata(OutputFLV, 0.0, 0.0);
    filesize += pretagsize;

    //write previous tag size
    filesize += flvmuxer_writepretagsize(OutputFLV, pretagsize);

    //write AAC sequence header
    char adtsheader[AAC_ADTS_HEADER_SIZE] = {0};
    read(AAC, &adtsheader,AAC_ADTS_HEADER_SIZE);
    pretagsize = flvmuxer_writeaudio_AudioSpecificConfig(OutputFLV, adtsheader);
    filesize += pretagsize;

    printf("Audio AudioSpecificConfig size : %d, ts : %f \n", pretagsize, audiotimestamp);

    //write previous tag size
    pretagsize += flvmuxer_writepretagsize(OutputFLV, pretagsize);
    filesize += pretagsize;

    //write AVC sequence header
    char *sps, *pps;
    int sps_size, pps_size;
    flvmuxer_readspspps(H264, &sps, &sps_size, &pps, &pps_size);
    pretagsize = flvmuxer_writevideo_AVCDecoderConfigurationRecord(OutputFLV, sps, sps_size, pps, pps_size);
    filesize += pretagsize;
    FREEIF(sps);
    FREEIF(pps);
    printf("Video AVCDecoderConfigurationRecord size : %d, ts : %f \n", pretagsize, audiotimestamp);

    //write previous tag size
    pretagsize += flvmuxer_writepretagsize(OutputFLV, pretagsize);
    filesize += pretagsize;

    for(;;)
    {
        //interleave audio&video
        if(audiotimestamp > videotimestamp)
        {
            //write AVC frame
            /*----------------------------------------------------------*/
            char *AVCframe = NULL; 
            int AVCframesize = 0;
            flvmuxer_readAVCframe(H264, AVCpos, H264filesize, &AVCframe, &AVCframesize);
            if(AVCframe == NULL || AVCframesize == 0)
            {
                break;
            }
            AVCpos += AVCframesize;
            AvcNaluType nut = (AvcNaluType)(AVCframe[4] & 0x1f);
            if(nut == AvcNaluTypeSPS 
                || nut == AvcNaluTypePPS)
            {
                FREEIF(AVCframe);
                continue;
            }
            NALU_t *nalu = AllocNalu(AVC_FRAMESIZE_MAX);
            flvmuv_getAnnexbNALU((unsigned char *)AVCframe, (unsigned int)AVCframesize, nalu);
            FreeNalu(nalu);
            pretagsize = flvmuxer_writevideodata(OutputFLV, AVCframe, AVCframesize, videotimestamp);
            videotimestamp += 1000/30.00;
            filesize += pretagsize;
            FREEIF(AVCframe);
            /*----------------------------------------------------------*/
            //write previous tag size
            /*----------------------------------------------------------*/

            filesize += flvmuxer_writepretagsize(OutputFLV, pretagsize);
            /*----------------------------------------------------------*/
            printf("VIDEO-TAG size : %d, timestamp : %f \n", pretagsize, videotimestamp);
        }
        else
        {
            //write AAC frame
            /*----------------------------------------------------------*/
            char *AACframe = NULL; 
            int AACframesize = 0;
            flvmuxer_readAACframe(AAC, AACpos, AACfilesize, &AACframe, &AACframesize);
            if(AACframe == NULL || AACframesize == 0)
            {
                break;
            }
            AACpos += AACframesize;
            pretagsize = flvmuxer_writeaudiodata(OutputFLV, AACframe, AACframesize, audiotimestamp);
            audiotimestamp += 1024*1000/44100.00;
            filesize += pretagsize;
            FREEIF(AACframe);
            /*----------------------------------------------------------*/

            //write previous tag size
            /*----------------------------------------------------------*/
            filesize += flvmuxer_writepretagsize(OutputFLV, pretagsize);
            /*----------------------------------------------------------*/
            printf("AUDIO-TAG size : %d, timestamp : %f \n", pretagsize, audiotimestamp);
        }
    }

    seek(OutputFLV, FLV_HEADER_SIZE + FLV_PREVIOUS_TAG_SIZE, SEEK_SET);
    flvmuxer_writesriptdata(OutputFLV, audiotimestamp/1000.0, filesize);
    seek(OutputFLV, 0, SEEK_END);

    close(H264);
    close(AAC);
    close(OutputFLV);
    printf("=============process end=============\n");
    system("pause");
    return 0;
}