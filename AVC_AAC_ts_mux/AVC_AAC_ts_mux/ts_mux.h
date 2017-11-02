#pragma once
#include "fileio.h"

#define AVCFILE "E:\\sourcecode\\visualstudio2010\\Projects\\AVC_STREAM\\FourPeople_720p_2K.h264"
#define AACFILE "E:\\sourcecode\\visualstudio2010\\Projects\\AAC_STREAM\\Audio_aac_2ch_152kbps_44100hz-000603.aac"
#define OUTPUTTS "E:\\sourcecode\\visualstudio2010\\Projects\\MUX_STREAM\\output.ts"

extern unsigned int write_ts_pat_packet(FILE *fd);

extern unsigned int write_ts_pmt_packet(FILE *fd);