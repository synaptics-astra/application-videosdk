#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include "v4l2_api.h"
#include "v4l2hdmiExt.h"


FILE *ofp = NULL;
void
print_usage ()
{
    printf ("\nUsage: ./v4l2hdmiExt-test -w <width> -h <height>\n");
    return;
}

void *
DqueueThread (void *arg)
{

    printf (">>>>> Fn(%s)\n", __func__);
    v4l2hdmiExt *hdmiext = (v4l2hdmiExt *) arg;
    struct v4l2_buffer buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    int ret = V4L2_SUCCESS;
    long size = 0;

    while (!hdmiext->EndOfStream) {
        memset (&buf, 0, sizeof (buf));
        memset (&planes, 0, sizeof(planes));
        buf.m.planes = planes;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = SynaV4L2_Ioctl (hdmiext->handle, VIDIOC_DQBUF, &buf);
        if (ret != V4L2_SUCCESS) {
            printf ("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__,
                __LINE__);
            goto error;
        } else {
            //Dump data;
            size = ftell(ofp);
            if(size > 500*1024*1024*1024)
                rewind(ofp);

            ret = fwrite (hdmiext->mmap_virtual_output[buf.index], 1, hdmiext->mmap_size_output[buf.index], ofp);
            fflush (ofp);
            buf.bytesused = 0;
            ret = SynaV4L2_Ioctl (hdmiext->handle, VIDIOC_QBUF, &buf);
            if (ret != V4L2_SUCCESS) {
                printf ("%s: %d ERROR: VIDIOC_QBUF Failed\n", __func__,
                    __LINE__);
                goto error;
            }
        }
    }

  error:
    printf ("<<<<< Fn(%s)\n", __func__);
    return NULL;
}


int
main (int argc, char *argv[])
{
    int opt;
    int ret = V4L2_SUCCESS;
    v4l2hdmiExt *hdmiext;
    pthread_t tid;
    char outputFileName[64] = { 0 };

    hdmiext = (v4l2hdmiExt *) malloc (sizeof (v4l2hdmiExt));
    memset (hdmiext, 0, sizeof (v4l2hdmiExt));

    /*Set default width and Height */
    hdmiext->width = 1920;
    hdmiext->height = 1080;
    hdmiext->pixelformat = V4L2_PIX_FMT_NV12;
    printf ("Date: 1st Aug 2023\n");


    while ((opt = getopt (argc, argv, ":w:h:")) != -1) {
        switch (opt) {
          case 'w':
              hdmiext->width = atoi (optarg);
              printf ("width %d\n", hdmiext->width);
              break;

          case 'h':
              hdmiext->height = atoi (optarg);
              printf ("height %d\n", hdmiext->height);
              break;

          default:
              printf ("unknown option: %c \n", opt);
              print_usage ();
              exit (0);
        }
    }

    /*Open Output file to dump down scaled hdmi-rx frames */
    sprintf (outputFileName, "/tmp/hdmirx_%dx%d_out.nv12", hdmiext->width,
        hdmiext->height);
    ofp = fopen (outputFileName, "wb");
    if (!ofp) {
        printf ("error: Unable to open output file\n");
        goto err;
    } else {
        printf ("File path to dump(%s)\n", outputFileName);
    }


    // Initialize V4l2 library
    ret = SynaV4L2_Init ();
    if (ret != V4L2_SUCCESS) {
        printf ("Error in v4l2 init\n");
        goto err;
    }
    //init and get hdmiext component handle
    ret =
        SynaV4L2_Open (&hdmiext->handle, HDMI_RX_EXT, hdmiext->pixelformat,
        O_NONBLOCK);
    if (ret == V4L2_SUCCESS) {
        printf ("v4l2hdmiExt open done handle = 0x%x\n", (int) hdmiext->handle);
    } else {
        printf ("Error in SynaV4L2_Open\n");
        goto err;
    }

    //set output pixal format, width and height
    if (v4l2hdmiExt_set_output (hdmiext) < 0) {
        printf ("v4l2hdmiExt_set_formats failed\n");
        goto err;
    }


    /* Request output buffers */
    if (v4l2hdmiExt_req_output_buf (hdmiext) < 0) {
        printf ("v4l2dns_req_output_buf failed\n");
        goto err;
    }

    /* Stream On CAPTURE */
    /* Note: Stream On before requesting the capture buffer */
    ret = v4l2hdmiExt_streamOn (hdmiext);
    if (ret != V4L2_SUCCESS) {
        printf ("%s: capture vidioc_streamon failed\n", __func__);
    } else {
        printf ("Started streamon for CAPTURE..\n");
    }

    /* Create a thread to Dqueue the output buffers */
    pthread_create (&tid, NULL, DqueueThread, (void *) hdmiext);
    printf ("Starting the main loop to feed input data\n");



    /*Wait to join the thread */
    pthread_join (tid, NULL);

    /* Free the resources */
    ret = SynaV4L2_Close (hdmiext->handle);
    if (ret != V4L2_SUCCESS) {
        printf ("SynaV4L2_Close Failed\n");
    }
    ret = SynaV4L2_Deinit ();
    if (ret != V4L2_SUCCESS) {
        printf ("SynaV4L2_Deinit Failed\n");
    }

  err:
    if (ofp)
        fclose (ofp);
    free (hdmiext);

    printf ("*** closed V4l2 HDMIEXT session..\n");
    return 0;
}
