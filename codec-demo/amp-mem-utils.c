#include "amp-mem-utils.h"

#define AMP_SHM_DEV "/dev/amp-shm"

typedef struct __v4l2_context__ {
	UINT32 inited;
	AMP_FACTORY amp_factory;
	UINT32 flag;
	UINT32 refcnt;
} V4L2_CONTEXT;

static int amp_shm_fd = -1;
static char *client_argv[] = { (char *)"client",
				(char *)"iiop:1.0//127.0.0.1:999/AMP::FACTORY/factory" };
static V4L2_CONTEXT context = {0};

AMPBufInfo* allocateAMPCQBuf(unsigned int size){

	int ret = 0;
	AMPBufInfo *newMem = (AMPBufInfo *)malloc(sizeof(AMPBufInfo));

	if(newMem == NULL)
		goto out;

	if(amp_shm_fd == -1){
		MV_OSAL_Init();

		AMP_Initialize(2, client_argv, &(context.amp_factory));

		amp_shm_fd = open(AMP_SHM_DEV, O_RDWR, 0);

		if(amp_shm_fd == -1){
			perror("amp_shm_fd");
			free(newMem);
			newMem = NULL;
			goto out;
		}
	}

	ret = AMP_SHM_Allocate(AMP_SHM_NONSECURE, size, 1024, &newMem->hShm);

	if(ret != 0){
		free(newMem);
		LOG_DEF("SYNA: AMP_SHM_Allocate for size(%d) failed ret(%d)\n", size, ret);
		goto out;
	}

	AMP_SHM_GetPhysicalAddress(newMem->hShm, 0, (void **)&newMem->phyAddr);
	AMP_SHM_GetVirtualAddress(newMem->hShm, 0, (void **)&newMem->vAddr);

	ret = AMP_SHM_Share(newMem->hShm, &newMem->mShareFd);

	if (ret == SUCCESS) {
		LOG_DEF("Success to share AMPCQBuf (handle=0x%x) mShareFd(%d)\n", newMem->hShm, newMem->mShareFd);
	} else {
		LOG_DEF("Failed to share AMPCQBuf with error[0x%x]\n", ret);
	}


out:
	return newMem;
}

int allocateAMPBuf(AMPBufInfo **pAMPBufInfo, int nCnt, int size ) {

	int ret = 0, i;
		for(i = 0; i < nCnt; i++) {
				pAMPBufInfo[i] = allocateAMPCQBuf(size);
				if(pAMPBufInfo[i] == NULL){
						ret = -1;
						LOG_DEF("AMP-MEM-UTILS: ERROR:  failed\n");
						break;
				}else{
						LOG_DEF("AMP-MEM-UTILS: shared_fd(%d) created\n", pAMPBufInfo[i]->mShareFd);
						//LOG_DEF("AMP-MEM-UTILS: pGrBufInfo[%d](%dx%d): phyAddr %.8x, vAddr %p, size :%d\n", i, width, height,
		}

	}
	return ret;
}
