#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include "v4l2_api.h"
#include "v4l2hdmiExt.h"


int
v4l2hdmiExt_streamOn (v4l2hdmiExt * hdmiext)
{

    int ret = V4L2_SUCCESS;
    V4L2_HANDLE handle = hdmiext->handle;
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    ret = SynaV4L2_Ioctl (handle, VIDIOC_STREAMON, &type);
    if (ret != V4L2_SUCCESS) {
        printf ("%s: %d ERROR: VIDIOC_STREAMON Failed\n", __func__, __LINE__);
    }
    return ret;
}


int
v4l2hdmiExt_streamOff (v4l2hdmiExt * hdmiext)
{
    int ret = V4L2_SUCCESS;
    V4L2_HANDLE handle = hdmiext->handle;
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    /* Stop streaming on output */
    ret = SynaV4L2_Ioctl (handle, VIDIOC_STREAMOFF, &type);
    if (ret != V4L2_SUCCESS) {
        printf ("ERROR: VIDIOC_STREAMOFF Failed on CAPTURE\n");
    }
    return ret;
}


int
v4l2hdmiExt_set_output (v4l2hdmiExt * hdmiext)
{


    unsigned int ret = V4L2_SUCCESS;
    struct v4l2_format set_fmt;
    struct v4l2_format get_fmt;
    V4L2_HANDLE handle = hdmiext->handle;
    printf (">>>>>Fn(%s)\n", __func__);

    //Set Format for output
    memset (&set_fmt, 0, sizeof (set_fmt));
    set_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    set_fmt.fmt.pix_mp.width = hdmiext->width;
    set_fmt.fmt.pix_mp.height = hdmiext->height;
    set_fmt.fmt.pix_mp.pixelformat = hdmiext->pixelformat;
    printf ("In Fn(%s): Setting width(%d) height(%d) pixelformat(%d)\n",
        __func__, set_fmt.fmt.pix_mp.width, set_fmt.fmt.pix_mp.height,
        set_fmt.fmt.pix_mp.pixelformat);

    ret = SynaV4L2_Ioctl (handle, VIDIOC_S_FMT, &set_fmt);
    if (ret != V4L2_SUCCESS) {
        printf ("Error: %s: Output VIDIOC_S_FMT failed\n", __func__);
        goto error;
    }

    memset (&get_fmt, 0, sizeof (get_fmt));
    get_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ret = SynaV4L2_Ioctl (handle, VIDIOC_G_FMT, &get_fmt);
    if (ret != V4L2_SUCCESS) {
        printf ("Error: %s: Output VIDIOC_G_FMT failed\n", __func__);
    }
  error:
    printf ("<<<<<Fn(%s)\n", __func__);
    return ret;
}


int
v4l2hdmiExt_req_output_buf (v4l2hdmiExt * hdmiext)
{

    int ret = V4L2_SUCCESS;
    struct v4l2_requestbuffers reqbuf;
    V4L2_HANDLE handle = hdmiext->handle;

    struct v4l2_buffer querybuf;
    struct v4l2_plane queryplanes[VIDEO_MAX_PLANES];
    void *m_output_ptr = NULL;
    int index;
    printf (">>>>>Fn(%s)\n", __func__);


    /* Memory mapping for input buffers in V4L2 */
    memset (&reqbuf, 0, sizeof reqbuf);
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    ret = SynaV4L2_Ioctl (handle, VIDIOC_REQBUFS, &reqbuf);
    if (ret != V4L2_SUCCESS) {
        printf ("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__, __LINE__);
        goto error;
    }

    hdmiext->current_nb_buf_output = reqbuf.count;
    printf ("VIDIOC_REQBUFS: reqbuf.count(%d)\n",
        hdmiext->current_nb_buf_output);
    hdmiext->mmap_virtual_output =
        malloc (sizeof (void *) * hdmiext->current_nb_buf_output);
    hdmiext->mmap_size_output =
        malloc (sizeof (void *) * hdmiext->current_nb_buf_output);

    /*Query output buffers, mmap and queue them to driver */
    for (index = 0; index < hdmiext->current_nb_buf_output; index++) {

        memset (&querybuf, 0, sizeof (querybuf));
        memset (&queryplanes, 0, sizeof (queryplanes));
        querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        querybuf.memory = V4L2_MEMORY_MMAP;
        querybuf.m.planes = queryplanes;
        querybuf.index = index;
        ret = SynaV4L2_Ioctl (handle, VIDIOC_QUERYBUF, &querybuf);
        if (ret != V4L2_SUCCESS) {
            printf ("%s: %d ERROR: VIDIOC_QUERYBUF Failed for index(%d)\n",
                __func__, __LINE__, index);
            ret = -1;
            break;
        }

        m_output_ptr = SynaV4L2_Mmap (handle, NULL, querybuf.m.planes[0].length,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            querybuf.m.planes[0].m.mem_offset);

        if (m_output_ptr == NULL) {
            printf ("%s: %d ERROR: SynaV4L2_Mmap Failed for index(%d)\n",
                __func__, __LINE__, index);
            ret = -1;
            break;
        } else {
            printf
                ("%s: Successfully mapped output memory for index(%d) VA(%p) Len(%d)\n",
                __func__, index, m_output_ptr, querybuf.m.planes[0].length);
        }

        hdmiext->mmap_virtual_output[index] = m_output_ptr;
        hdmiext->mmap_size_output[index] = querybuf.m.planes[0].length;

    }
  error:

    printf ("<<<<<Fn(%s)\n", __func__);
    return ret;
}



int
v4l2hdmiExt_qbuf (v4l2hdmiExt * hdmiext)
{

    int ret = V4L2_SUCCESS;
    V4L2_HANDLE handle = hdmiext->handle;
    struct v4l2_buffer qbuf;
    memset (&qbuf, 0, sizeof (qbuf));
    printf (">>>>>Fn(%s)\n", __func__);

    qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    qbuf.m.planes[0].bytesused = 0;
    ret = SynaV4L2_Ioctl (handle, VIDIOC_QBUF, &qbuf);
    if (ret != 0) {
        printf ("VIDIOC_QBUF failed = (%d) @Line(%d) in Fn(%s)\n", ret,
            __LINE__, __func__);
        ret = -1;
    }
    printf ("<<<<<Fn(%s)\n", __func__);

    return ret;

}


int
v4l2hdmiExt_dqbuf (v4l2hdmiExt * hdmiext)
{

    int ret = V4L2_SUCCESS;
    V4L2_HANDLE handle = hdmiext->handle;
    struct v4l2_buffer dqbuf;
    memset (&dqbuf, 0, sizeof (dqbuf));
    printf (">>>>>Fn(%s)\n", __func__);

    dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ret = SynaV4L2_Ioctl (handle, VIDIOC_DQBUF, &dqbuf);
    if (ret != 0) {
        printf ("VIDIOC_QBUF failed = (%d) @Line(%d) in Fn(%s)\n", ret,
            __LINE__, __func__);
        ret = -1;
    }
    printf ("<<<<<Fn(%s)\n", __func__);
    return ret;
}


void
unmap_output_buf (v4l2hdmiExt * hdmiext)
{

    int i;
    V4L2_HANDLE handle = hdmiext->handle;

    if (hdmiext->mmap_virtual_output) {
        for (i = 0; i < hdmiext->current_nb_buf_output; i++)
            SynaV4L2_Munmap (handle, hdmiext->mmap_virtual_output[i],
                hdmiext->mmap_size_output[i]);
    }
}

int
v4l2hdmiExt_close (v4l2hdmiExt * hdmiext)
{

    int type;
    int ret = V4L2_SUCCESS;
    V4L2_HANDLE handle = hdmiext->handle;

    hdmiext->EndOfStream = 1;

    /* Stop streaming on output */
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ret = SynaV4L2_Ioctl (handle, VIDIOC_STREAMOFF, &type);
    if (ret != V4L2_SUCCESS) {
        printf ("ERROR: VIDIOC_STREAMOFF Failed on CAPTURE\n");
        return -1;
    }

    unmap_output_buf (hdmiext);

    if (hdmiext->mmap_virtual_output) {
        free (hdmiext->mmap_virtual_output);
        free (hdmiext->mmap_size_output);
    }

    return 0;
}


void *
v4l2_event_watcher (void *data)
{

    v4l2hdmiExt *hdmiext = (v4l2hdmiExt *) data;
    V4L2_HANDLE handle = hdmiext->handle;
    int ret = 0;
    struct v4l2_event dqevent;

    while (!hdmiext->EndOfStream) {

        memset (&dqevent, 0, sizeof (struct v4l2_event));
        ret = SynaV4L2_Ioctl (handle, VIDIOC_DQEVENT, &dqevent);
        if (ret != V4L2_SUCCESS) {
            usleep (30000);
            continue;
        }

        switch (dqevent.type) {
          case V4L2_EVENT_SOURCE_CHANGE:
              printf ("case: V4L2_EVENT_SOURCE_CHANGE\n");
              break;

          default:
              printf ("Unknown EventType(%d)\n", dqevent.type);
        }
    }

    return NULL;
}
