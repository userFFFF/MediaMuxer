#pragma once

typedef struct _AAC_Config_
{
    unsigned char audioObjectType;              //5;��������ͣ�AAC-LC = 0x02
    unsigned char samplingFrequencyIndex;       //4;������ 44100 = 0x04
    unsigned char channelConfiguration;         //4;���� = 2
    unsigned char framelengthFlag;              //1;��־λ��λ�ڱ���IMDCT���ڳ��� = 0
    unsigned char dependsOnCoreCoder;           //1;��־λ�������Ƿ�������corecoder = 0
    unsigned char extensionFlag;                //1;ѡ����AAC-LC = 0
}aac_specificconfig_t;

typedef struct _ADTS_HEADER_
{
	unsigned int syncword;  //12 bslbf ͬ����The bit string ��1111 1111 1111����˵��һ��ADTS֡�Ŀ�ʼ
	unsigned int id;        //1 bslbf   MPEG ��ʾ��, ����Ϊ1
	unsigned int layer;     //2 uimsbf Indicates which layer is used. Set to ��00��
	unsigned int protection_absent;  //1 bslbf  ��ʾ�Ƿ�����У��
	unsigned int profile;            //2 uimsbf  ��ʾʹ���ĸ������AAC����01 Low Complexity(LC)--- AACLC
	unsigned int sf_index;           //4 uimsbf  ��ʾʹ�õĲ������±�
	unsigned int private_bit;        //1 bslbf 
	unsigned int channel_configuration;  //3 uimsbf  ��ʾ������
	unsigned int original;               //1 bslbf 
	unsigned int home;                   //1 bslbf 
	/*�����Ϊ�ı�Ĳ�����ÿһ֡����ͬ*/
	unsigned int copyright_identification_bit;   //1 bslbf 
	unsigned int copyright_identification_start; //1 bslbf
	unsigned int aac_frame_length;               // 13 bslbf  һ��ADTS֡�ĳ��Ȱ���ADTSͷ��raw data block
	unsigned int adts_buffer_fullness;           //11 bslbf     0x7FF ˵�������ʿɱ������
	/*no_raw_data_blocks_in_frame ��ʾADTS֡����number_of_raw_data_blocks_in_frame + 1��AACԭʼ֡.
	����˵number_of_raw_data_blocks_in_frame == 0 
	��ʾ˵ADTS֡����һ��AAC���ݿ鲢����˵û�С�(һ��AACԭʼ֡����һ��ʱ����1024���������������)
    */
	unsigned int no_raw_data_blocks_in_frame;    //2 uimsfb
}adts_header_t;

adts_header_t *Parser_AAC_ADTSHeader(unsigned char *Adts_Headr_Buf);

aac_specificconfig_t Gen_AAC_SpecificConfig(unsigned char *adtsheader);

unsigned int GetAACFrame(unsigned char *buffer, unsigned char **frame, unsigned int *framesize);