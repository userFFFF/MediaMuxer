#include "StdAfx.h"
#include <stdlib.h>
#include "bs.h"
#include "avc_utils.h"
#include "common.h"

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

int GetFrametype(NALU_t *nal)
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
        printf("AVC READ ERROR£¡\n");
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

unsigned int GetAnnexbNALU(unsigned char *buffer, unsigned int size, NALU_t *nalu)
{    
    unsigned int pos = 0;//get next Annexb Nalu position
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

    return nalu->startcodeprefix_len;
}