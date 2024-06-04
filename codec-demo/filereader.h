// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include "config.h"

typedef struct filereader_t
{
	// File Reader data
	enc_src_type_e		src;
	int					width;
	int					height;
	char 				path[MAX_FILE_PATH_STR];
	FILE				*readFile;
	int					fps;
	long				frame_duration_us;
	struct timeval		time_stamp;
	char				*readPtr;
}filereader_t;

int SetupFileReader(filereader_t *pFileReader);
void ReadFrameFromFile(filereader_t *pFileReader, void* ptr, int *size);
void ShutdownFileReader(filereader_t *pFileReader);
void SleepForMaintainingFps(filereader_t *pFileReader);

#endif //_FILEREADER_H_
