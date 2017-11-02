#ifndef _AMF_
#define _AMF_

enum MFDataType
{
    AMF_NUMBER = 0, AMF_BOOLEAN, AMF_STRING, AMF_OBJECT,
    AMF_MOVIECLIP,        /* reserved, not used */
    AMF_NULL, AMF_UNDEFINED, AMF_REFERENCE, AMF_ECMA_ARRAY, AMF_OBJECT_END,
    AMF_STRICT_ARRAY, AMF_DATE, AMF_LONG_STRING, AMF_UNSUPPORTED,
    AMF_RECORDSET,        /* reserved, not used */
    AMF_XML_DOC, AMF_TYPED_OBJECT,
    AMF_AVMPLUS,        /* switch to AMF3 */
    AMF_INVALID = 0xff
};

char *put_byte(char *out_chPut, unsigned char in_uchVal);
char *put_be16(char *out_chPut, unsigned short in_usVal);
char *put_be24(char *out_chPut, unsigned short in_usVal);
char *put_be32(char *out_chPut, unsigned int in_unVal);
char *put_amf_string(char *out_chPut, const char *in_chBuf);
char *put_amf_double(char *out_chPut, double in_dBuf);

#endif