#pragma once

typedef struct _AAC_Config_
{
    unsigned char audioObjectType;              //5;编解码类型：AAC-LC = 0x02
    unsigned char samplingFrequencyIndex;       //4;采样率 44100 = 0x04
    unsigned char channelConfiguration;         //4;声道 = 2
    unsigned char framelengthFlag;              //1;标志位，位于表明IMDCT窗口长度 = 0
    unsigned char dependsOnCoreCoder;           //1;标志位，表明是否依赖于corecoder = 0
    unsigned char extensionFlag;                //1;选择了AAC-LC = 0
}aac_specificconfig_t;

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

adts_header_t *Parser_AAC_ADTSHeader(unsigned char *Adts_Headr_Buf);

aac_specificconfig_t Gen_AAC_SpecificConfig(unsigned char *adtsheader);

unsigned int GetAACFrame(unsigned char *buffer, unsigned char **frame, unsigned int *framesize);