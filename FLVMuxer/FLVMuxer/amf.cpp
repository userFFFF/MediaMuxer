#include "stdafx.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "amf.h"

char *put_byte(char *out_chPut, unsigned char in_uchVal)
{
    out_chPut[0] = in_uchVal;
    return out_chPut+1;
}

char *put_be16(char *out_chPut, unsigned short in_usVal)
{
    out_chPut[0] = ((in_usVal) >> 8) & 0xFF;
    out_chPut[1] = in_usVal & 0xFF; 

    return out_chPut+2;
}

char *put_be24(char *out_chPut, unsigned short in_usVal)
{
    out_chPut[0] = ((in_usVal) >> 16) & 0xFF;
    out_chPut[1] = ((in_usVal) >> 8) & 0xFF;
    out_chPut[2] = in_usVal & 0xFF; 

    return out_chPut+3;
}

char *put_be32(char *out_chPut, unsigned int in_unVal)
{
    out_chPut[0] = (in_unVal) >> 24;
    out_chPut[1] = ((in_unVal) >> 16) & 0xFF;
    out_chPut[2] = ((in_unVal) >> 8) & 0xFF; 
    out_chPut[3] = in_unVal & 0xFF;
    return out_chPut+4;
}

char *put_amf_string(char *out_chPut, const char *in_chBuf)
{
    uint16_t len = (uint16_t)strlen( in_chBuf );
    out_chPut=put_be16( out_chPut, len );
    memcpy(out_chPut,in_chBuf,len);
    return out_chPut+len;
}

char *put_amf_double(char *out_chPut, double in_dBuf)
{
    *out_chPut++ = AMF_NUMBER;    // type: Number
    {
        unsigned char *ci, *co;
        ci = (unsigned char *)&in_dBuf;
        co = (unsigned char *)out_chPut;
        co[0] = ci[7];
        co[1] = ci[6];
        co[2] = ci[5];
        co[3] = ci[4];
        co[4] = ci[3];
        co[5] = ci[2];
        co[6] = ci[1];
        co[7] = ci[0];
    }
    return out_chPut+8;
}