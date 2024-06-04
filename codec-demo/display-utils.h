// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _DISPLAY_UTILS_H_
#define _DISPLAY_UTILS_H_
#include "syna_std_headers.h"

void SetupDisplayWindow(int display_window);
void PrepareDisplayBuffer(int index, unsigned int width, unsigned int height, void *virt_addr, unsigned long phy_addr, int share_fd, VOUT_BUFFER *buf);
void RenderDisplayBuffer(int display_window, VOUT_BUFFER *buf);
void ShutdownDisplayWindow(int display_window);

#endif //_DISPLAY_UTILS_H_
