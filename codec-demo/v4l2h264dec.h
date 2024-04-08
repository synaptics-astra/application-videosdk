#ifndef _V4L2H264DEC_H_
#define _V4L2H264DEC_H_

#include "decoder_interface_v4l2.h"

#define LIBSYNAV4L2_DECODER_INDEX 0

int SetupV4L2H264Dec(dec_config_t *pCfg);
int GetV4L2H264DecFreeInputBuffer(dec_config_t *pCfg, int *index);
int FeedBufferToV4L2H264Decoder(dec_config_t *pCfg);
void StopV4L2H264Decoder(dec_config_t *pCfg);
void ShutdownV4L2H264Decoder(dec_config_t *pCfg);
int StreamOffPort(dec_config_t *pCfg, unsigned int type);

#endif //_V4L2H264DEC_H_
