#pragma once

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
typedef enum _frametype_e
{
    FRAME_I  = 15,
    FRAME_P  = 16,
    FRAME_B  = 17
}frametype_t;

typedef struct _Video_AvcC
{
    unsigned char configurationVersion;  //8��= 0x01
    unsigned char AVCProfileIndication;  //sps��sps�ĵ�2�ֽ�,��ν��AVCProfileIndication
    unsigned char profile_compatibility; //sps��sps�ĵ�3�ֽ�,��ν��profile_compatibility
    unsigned char AVCLevelIndication;    //sps��sps�ĵ�4�ֽ�,��ν��AVCLevelIndication
    unsigned char reserved_1;            //��111111��b;
    unsigned char lengthSizeMinusOne;    //NALUnitLength �ĳ��� -1 һ��Ϊ0x03
    unsigned char reserved_2;            //��111��b;
    unsigned char numOfSequenceParameterSets;  //һ�㶼��һ��
    unsigned int sequenceParameterSetLength;   //sps����
    unsigned char *sequenceParameterSetNALUnit; //sps����
    unsigned char numOfPictureParameterSets;   //һ�㶼��һ��
    unsigned int  pictureParameterSetLength;   //pps����
    unsigned char *pictureParameterSetNALUnit;//pps����
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
    unsigned int  startcodeprefix_len;     //! ǰ׺�ֽ���
    unsigned int  len;                     //! ����nal ͷ��nal ���ȣ��ӵ�һ��00000001����һ��000000001�ĳ���
    unsigned int  max_size;                //! ����һ��nal �ĳ���
    unsigned char *buf;                   //! ����nal ͷ��nal ����
    unsigned char Frametype;               //! ֡����
    unsigned int  lost_packets;            //! Ԥ��
}NALU_t;

#include "StdAfx.h"
#include <stdlib.h>
#include "bs.h"
#include "avc_utils.h"
#include "common.h"

extern int avc_startcode2(unsigned char *buf);

extern int avc_startcode3(unsigned char *buf);

extern NALU_t *AllocNalu(int maxsize);

extern void FreeNalu(NALU_t *nalu);

extern int GetFrametype(NALU_t *nal);

extern unsigned int GetAnnexbNALU(unsigned char *buffer, unsigned int size, NALU_t *nalu);