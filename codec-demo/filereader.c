// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "filereader.h"

int SetupFileReader(filereader_t *pFileReader)
{
	pFileReader->readFile = fopen(pFileReader->path, "r");
	if (pFileReader->readFile == NULL)
	{
		LOG_ERR("No input file[%s] available\n", pFileReader->path);
		return -1;
	}

	pFileReader->frame_duration_us = (1000*1000)/pFileReader->fps;
	return 0;
}

void ReadFrameFromFile(filereader_t *pFileReader, void* ptr, int *size)
{
	int ret = 0;
	char *dstPtr = (char *) ptr;

	if(feof(pFileReader->readFile) != 0)
	{
		rewind(pFileReader->readFile);
	}

	ret = fread(dstPtr, 1, *size, pFileReader->readFile);

	if(ret < *size) {
		rewind(pFileReader->readFile);
		ret = fread(dstPtr, 1, *size, pFileReader->readFile);
	}
}

void ShutdownFileReader(filereader_t *pFileReader)
{
	if (pFileReader->readFile)
	{
		fclose(pFileReader->readFile);
	}
	if (pFileReader->readPtr != NULL)
	{
		free(pFileReader->readPtr);
	}
}

void SleepForMaintainingFps(filereader_t *pFileReader)
{
	struct timeval end;

	gettimeofday(&end, NULL);

	long seconds = (end.tv_sec - pFileReader->time_stamp.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (pFileReader->time_stamp.tv_usec);

	long usec_sleep = pFileReader->frame_duration_us - micros;

	if (usec_sleep > 0)
		usleep(usec_sleep);

	//LOG_DEF ("sleep for %ld\n", usec_sleep);
	gettimeofday(&pFileReader->time_stamp, NULL);
}
