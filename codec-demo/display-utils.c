#include "display-utils.h"

// Window 1 -> Plane V1
static int plane1 = VO_WRAP_VIDEO_PLANE_V1;
static int w1 = 600;
static int h1 = 512;
static int x1 = 0;
static int y1 = 0;

// Window 2 -> Plane V2
static int plane2 = VO_WRAP_VIDEO_PLANE_V2;
static int w2 = 600;
static int h2 = 512;
static int x2 = 0;
static int y2 = 512;

#define MAXLINE	1000
#define HDMI_TX_INFO_FILE "/sys/devices/platform/9800d000.hdmitx/hdmitx_info"
char const* findkey="Resolution: ";

void getHdmiTxInfo()
{
	FILE *fcfg = NULL;
	char line[MAXLINE];
	char *p,*pend;
	int findlen;
	int width = 0, height = 0;
	int displayWidth = 1920;
	int displayHeight = 1080;

	findlen=strlen(findkey);
	fcfg=fopen(HDMI_TX_INFO_FILE,"r");

	if(fcfg == NULL){
		LOG_DEF("File(%s) failed to open\n", HDMI_TX_INFO_FILE);
		return;
	}

	while (p=fgets(line, MAXLINE, fcfg)) {
		//LOG_DEF("Looking at %s\n",p);
		if(strncmp(line,findkey,findlen)==0)
		{
			sscanf(line,"Resolution: %dx%dP",&width,&height);
			LOG_DEF("HDMI TX resolution: %dx%d\n", width, height);
			if (width != 0 && height != 0)
			{
				displayWidth = width;
				displayHeight = height;
			}
			break;
		}
	}

	if(fcfg != NULL)
		fclose(fcfg);

	// Modify Window content
	w1 = displayWidth >> 1;
	h1 = displayHeight >> 1;
	w2 = displayWidth >> 1;
	h2 = displayHeight >> 1;

	x2 = displayWidth >> 1;
}

void SetupDisplayWindow(int display_window)
{
	getHdmiTxInfo();
	if (display_window == 1)
	{
		vo_wrap_init(plane1);
		vo_wrap_set_display_window(plane1, x1, y1, w1, h1);
	}
	else if (display_window == 2)
	{
		vo_wrap_init(plane2);
		vo_wrap_set_display_window(plane2, x2, y2, w2, h2);		
	}
}

void PrepareDisplayBuffer(int index, unsigned int width, unsigned int height, void *virt_addr, unsigned long phy_addr, int share_fd, VOUT_BUFFER *buf)
{
	memset(buf,0,sizeof(VOUT_BUFFER));
	buf->width = buf->stride = ROUNDUP16(width);
	buf->height = ROUNDUP16(height);
	buf->slice_height = ROUNDUP16(height);
	buf->v1_wIndex = index;
	buf->pixel_format =  HAL_PIXEL_FORMAT_DSPG_W16_H16_YCbCr_420_SP;
	buf->m_dspg_data.delay_mode = 6;//LOW_DELAY_DISPLAY_ORDER
	buf->m_dspg_data.yuv_mode = 4;//CONSECUTIVE_FRAME
	buf->m_dspg_data.deintflag = 2;
	//buf->m_dspg_data.IsForceDIBobMode = 0;
	buf->m_dspg_data.lumaOffTblAddr = 0xffffffff;
	buf->m_dspg_data.chromaOffTblAddr = 0xffffffff;
	buf->m_dspg_data.bufBitDepth = 8;
	buf->m_dspg_data.bufFormat = 0;
	buf->m_dspg_data.display_width = width;
	buf->m_dspg_data.display_height = height;
	buf->m_dspg_data.display_stride = width;
	//buf->m_dspg_data.transferCharacteristics = 0;
	buf->virt_addr = virt_addr;
	buf->phys_addr = phy_addr;
	buf->share_fd = share_fd;
}

void RenderDisplayBuffer(int display_window, VOUT_BUFFER *buf)
{
	if (display_window == 1)
	{
		vo_wrap_render(plane1, buf);
	}
	else if (display_window == 2)
	{
		vo_wrap_render(plane2, buf);
	}
}

void ShutdownDisplayWindow(int display_window)
{
	if (display_window == 1)
	{
		vo_wrap_v1_stop(plane1);
		vo_wrap_deinit(plane1);
	}
	else if (display_window == 2)
	{
		vo_wrap_v1_stop(plane2);
		vo_wrap_deinit(plane2);
	}
}
