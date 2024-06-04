// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "config.h"

#include "decoder_interface_sw.h"
#include "encoder_interface_sw.h"
#ifdef V4L2_SUPPORT
#include "encoder_interface_hwv4l2.h"
#include "decoder_interface_hwv4l2.h"
#endif

void startInstance(sess_config_t *sess_cfg);
void stopInstance(sess_config_t *sess_cfg);
void waitForInstanceShutdown(sess_config_t *sess_cfg);

#endif // _INSTANCE_H_
