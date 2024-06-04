// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _MAIN_H_
#define _MAIN_H_
#include "syna_std_headers.h"

#include "config.h"
#include "instance.h"
#include "UIInterface.h"

#include "encoder_interface_sw.h"
#include "decoder_interface_sw.h"

#ifdef V4L2_SUPPORT
#include "encoder_interface_hwv4l2.h"
#include "decoder_interface_hwv4l2.h"
#endif

int	main_loop = 0;
tsem_t* mainLoopSem;

char config_file[256];
codec_config_t codec_config;

int srtp_enabled = 0;
int stats_enable_flag = 1;

#endif // _MAIN_H_
