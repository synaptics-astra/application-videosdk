#ifndef _GBM_MEM_UTILS_H_
#define _GBM_MEM_UTILS_H_
#include "syna_std_headers.h"

typedef struct _GBMBufInfo {
	void *		vAddr;
	unsigned int	size;
	void *		phyAddr;
	int		mShareFd;
	struct gbm_bo 	*gbm_bo;
} GBMBufInfo;

int InitGBM(char *device);
int AllocateDRMBuf(GBMBufInfo **pGBMBufInfo, int nCnt, int width, int height, int format);
int FreeDRMBuf(GBMBufInfo **pGBMBufInfo, int nCnt,int width, int height,int format);
void DeInitGBM();
#endif //_GBM_MEM_UTILS_H_
