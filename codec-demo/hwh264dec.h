// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _HW_V4L2H264DEC_H_
#define _HW_V4L2H264DEC_H_

#include "decoder_interface_hwv4l2.h"

#define LIBSYNAV4L2_DECODER_INDEX 0
#define HW_MAX_DEVICES 10

int SetupHWV4L2H264Dec(dec_config_t *pCfg);
int GetHWV4L2H264DecFreeInputBuffer(dec_config_t *pCfg, int *index);
int FeedBufferToHWV4L2H264Decoder(dec_config_t *pCfg);
void StopHWV4L2H264Decoder(dec_config_t *pCfg);
void ShutdownHWV4L2H264Decoder(dec_config_t *pCfg);
int StreamOffHWPort(dec_config_t *pCfg, unsigned int type);

#endif //_HW_V4L2H264DEC_H_
