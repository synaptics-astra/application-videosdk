// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "gbm-mem-utils.h"

static int drm_fd_;
static struct gbm_device *gbm;

int InitGBM(char *device)
{
	int ret = 0;
	if(!drm_fd_){
		drm_fd_ = open(device, O_RDWR | FD_CLOEXEC);
		if(drm_fd_ < 0){
			LOG_DEF("ERROR: Failed to open card(/dev/dri/card0)\n");
			return -1;
		}else{
			drmDropMaster(drm_fd_);
			gbm = gbm_create_device(drm_fd_);
			if(!gbm){
				LOG_DEF("ERROR: Failed gbm_create_device()\n");
				close(drm_fd_);
				drm_fd_ = 0;
				return -1;
			}
		}
	}
	return 0;
}
void DeInitGBM()
{
	if(gbm)
		gbm_device_destroy(gbm);

	gbm = NULL;
	if(drm_fd_)
		close(drm_fd_);
	drm_fd_ = 0;

	return;
}

GBMBufInfo* AllocateDRMCQBuf(int width, int height, int format) {

	GBMBufInfo *newMem = NULL;
	int device_fd = 0;
	uint32_t ePixelFormat;
	int size = 0;
	int ret = 0;


	newMem = (GBMBufInfo *)malloc(sizeof(GBMBufInfo));
	if(newMem == NULL){
		LOG_DEF("ERROR: malloc failed ");
			return NULL;
	}

	switch(format) {
		case GBM_FORMAT_NV12:
			size = width * height * 3 / 2;
			ePixelFormat = GBM_FORMAT_NV12;
			break;
		case GBM_FORMAT_ARGB8888:
			size = width * height * 4;
			ePixelFormat = GBM_FORMAT_ARGB8888;
			break;
		default:
			ret = -1;
		}

	if((ret == -1) || (!size)){
		LOG_DEF("Error: format(%d) size(%d)\n", format, size);
		return NULL;
	}

	newMem->gbm_bo = gbm_bo_create(gbm, width, height, ePixelFormat, GBM_BO_USE_RENDERING|GBM_BO_USE_SCANOUT);
	if (newMem->gbm_bo == NULL) {
		LOG_DEF("ERROR: while creating gbm bo\n");
		return NULL;
	}

	const union gbm_bo_handle handle = gbm_bo_get_handle_for_plane(newMem->gbm_bo, 0);
	device_fd = gbm_device_get_fd(gbm);
	ret = drmPrimeHandleToFD(device_fd, handle.u32, DRM_RDWR | DRM_CLOEXEC, &newMem->mShareFd);
	if (ret) {
			LOG_DEF ("ERROR: getting handle to prime fd\n");
			return NULL;
	} else {
		LOG_DEF ("drm handle fd from gbm : %d\n", newMem->mShareFd);
	}

	newMem->vAddr = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, newMem->mShareFd, 0);
	if (newMem->vAddr == MAP_FAILED) {
		LOG_DEF("ERROR: map failed.. \n");
		return NULL;
	}

	LOG_DEF("mapped virtual address : 0x%p\n", newMem->vAddr);
	return newMem;
}

int AllocateDRMBuf(GBMBufInfo **pGBMBufInfo, int nCnt, int width, int height, int format){

	int ret = 0, i;
	for(i = 0; i < nCnt; i++){
		pGBMBufInfo[i] = AllocateDRMCQBuf(width, height, format);
		if(pGBMBufInfo[i] == NULL){
			ret = -1;
			LOG_DEF("GBM-MEM-UTILS: ERROR:  allocateDRMCQBuf failed\n");
			break;
		}else{
			LOG_DEF("GBM-MEM-UTILS: shared_fd(%d) created\n", pGBMBufInfo[i]->mShareFd);
		}
	}
	return ret;
}
int FreeDRMBuf(GBMBufInfo **pGBMBufInfo, int nCnt,int width, int height,int format)
{
	int ret = 0, i,size;

	switch(format) {
		case GBM_FORMAT_NV12:
			size = width * height * 3 / 2;
			break;
		case GBM_FORMAT_ARGB8888:
			size = width * height * 4;
			break;
		default:
			LOG_DEF("GBM-MEM-UTILS: Not supported format(%d)\n", format);

	}

	for(i = 0; i < nCnt; i++){
		if(pGBMBufInfo[i] != NULL){

			if(pGBMBufInfo[i]->gbm_bo != NULL)
				gbm_bo_destroy(pGBMBufInfo[i]->gbm_bo);
			pGBMBufInfo[i]->gbm_bo = NULL;
			ret = munmap(pGBMBufInfo[i]->vAddr, size);
			if(ret != 0){
				LOG_ERR("GBM-MEM-UTILS: UnMapping Failed\n");
				return -1;
			}
			free(pGBMBufInfo[i]);
		}
	}
}
