#ifndef _AMP_MEM_UTILS_H_
#define _AMP_MEM_UTILS_H_
#include "syna_std_headers.h"

#include "amp_types.h"
#include "amp_malloc.h"
#include "isl/amp_logger.h"
#include "isl/amp_event_queue.h"
#include "isl/amp_shm.h"
#include "isl/amp_debug.h"

typedef struct _AMPBufInfo {
	AMP_SHM_HANDLE		hShm;
	void *			vAddr;
	unsigned int		size;
	void *			phyAddr;
	int			mShareFd;
} AMPBufInfo;

#endif //_AMP_MEM_UTILS_H_
