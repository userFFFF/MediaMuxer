#include "stdafx.h"
#include "aac_utils.h"

adts_header_t *Parser_AAC_ADTSHeader(unsigned char *Adts_Headr_Buf)
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


aac_specificconfig_t Gen_AAC_SpecificConfig(unsigned char *adtsheader)
{
    aac_specificconfig_t config = {0};
    if(adtsheader == NULL)
    {
        return config;
    }
    config.audioObjectType = (adtsheader[2] & 0xc0) >> 6;
    config.samplingFrequencyIndex = (adtsheader[2] & 0x3c) >> 2;
    config.channelConfiguration = (adtsheader[3] & 0xc0) >> 6;
    config.dependsOnCoreCoder = 0;
    config.extensionFlag = 0;
    config.framelengthFlag = 0;

    return config;
}

unsigned int GetAACFrame(unsigned char *buffer, unsigned char **frame, unsigned int *framesize)
{
    if (buffer[0] != (char)0xff || (char)(buffer[1] & 0xf0) != (char)0xf0) {
        *framesize = 0;
        return 0;
    }
    unsigned char ch3 = buffer[3];
    unsigned char ch4 = buffer[4];
    unsigned char ch5 = buffer[5];
    unsigned int size = ((ch3 << 11) & 0x1800) | ((ch4 << 3) & 0x07f8) | ((ch5 >> 5) & 0x0007);

    *frame = buffer;
    *framesize = size;
    return size;
}