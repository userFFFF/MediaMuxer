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