
ROOT_FS=$(DESTDIR)

rm=/bin/rm -f
cp=/bin/cp -f

MACH_DOLPHIN=dolphin
MACH_PLATYPUS=platypus
MACH_MYNA2=myna2

ifeq ($(CURR_MACH), $(MACH_DOLPHIN))
LOCAL_CFLAGS += -DCURR_MACH_DOLPHIN
else
ifeq ($(CURR_MACH), $(MACH_PLATYPUS))
LOCAL_CFLAGS += -DCURR_MACH_PLATYPUS
else
ifeq ($(CURR_MACH), $(MACH_MYNA2))
LOCAL_CFLAGS += -DCURR_MACH_MYNA2
endif
endif
endif

ifeq ($(V4L2_ENABLE), 1)
LOCAL_CFLAGS += -DV4L2_SUPPORT
endif

PROGNAME= codec-demo
VIDEOTUNERPROGNAME=video-tuner-app

LOCAL_C_INCLUDES += \
					. \
					$(SDKTARGETSYSROOT)/usr/include \
					${SDKTARGETSYSROOT}/usr/include/gstreamer-1.0 \
					${SDKTARGETSYSROOT}/usr/include/glib-2.0/ \
					${SDKTARGETSYSROOT}/usr/lib64/glib-2.0/include/ \
					${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/

LOCAL_SRC_FILES :=	main.c \
					tsemaphore.c \
					queue.c \
					config.c \
					instance.c \
					gsttxpipeline.c \
					filereader.c \
					gstrxpipeline.c \
					UIInterface.c \
					v4l2camera.c \
					encoder_interface_sw.c \
					swh264enc.c \
					decoder_interface_sw.c \
					swh264dec.c \
					gbm-mem-utils.c \
					neon_utils.c \

LIBS = \
	-lavformat \
	-lavcodec \
	-lavutil \
	-lswresample \
	-lpthread \
	-ldl -lz -lm -lbz2 \
	-lgbm -ldrm \
	-lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lgstapp-1.0

ifeq ($(V4L2_ENABLE), 1)
LOCAL_SRC_FILES +=	hwh264enc.c \
					encoder_interface_hwv4l2.c \
					hwh264dec.c \
					decoder_interface_hwv4l2.c \
					v4l2hrx.c
endif


OBJ_FILES = $(addsuffix .o, $(basename $(LOCAL_SRC_FILES)))

VIDEOTUNER_SRC_FILES=video_tuner_app.c
VT_OBJ_FILES = $(addsuffix .o, $(basename $(VIDEOTUNER_SRC_FILES)))

LOCAL_CFLAGS += -fPIC -g -DSYS_UNIX=1 -D_GNU_SOURCE

INCLUDES = $(addprefix -I, $(LOCAL_C_INCLUDES) )

LOCAL_WHOLE_LIBRARIES = \
-Wl,--no-whole-archive \

LIB_INCLUDES +=-L${SDKTARGETSYSROOT}/usr/lib64/:${SDKTARGETSYSROOT}/lib64/

CFLAGS += $(INCLUDES) $(DEFINES) $(LOCAL_CFLAGS) $(MYDEFS) -pthread

$(PROGNAME) : $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJ_FILES) -L${LIB_INCLUDES} $(LIBS) $(LOCAL_WHOLE_LIBRARIES)
	echo "codec-demo: Compiled successfully..!!"

$(VIDEOTUNERPROGNAME) : $(VT_OBJ_FILES)
	$(CC) $(CFLAGS) -o $(VIDEOTUNERPROGNAME) $(VT_OBJ_FILES) -L${LIB_INCLUDES} $(LIBS) $(LOCAL_WHOLE_LIBRARIES)
	echo "video-tuner-app: Compiled successfully..!!"


all: $(PROGNAME)

install:
	mkdir -p $(ROOT_FS)/usr/bin
	$(SHOW_COMMAND)$(cp) $(PROGNAME) $(ROOT_FS)/usr/bin

clean:
	$(SHOW_COMMAND)$(rm) $(OBJ_FILES) $(PROGNAME) *~ $(PROGNAME).debug
	$(SHOW_COMMAND)$(rm) $(ROOT_FS)/usr/bin/$(PROGNAME)

miniclean: clean

.cpp.o:
	$(rm) -f $@
	$(SHOW_COMMAND)echo -=--=--=- [$*.cpp] -=--=--=--=--=--=--=--=--=-
	$(CXX) $(CFLAGS) $(WARNING) $(DEBUGFLAG) -c $*.cpp -o *.h $@

.cc.o:
	$(rm) -f $@
	$(SHOW_COMMAND)echo -=--=--=- [$*.cc] -=--=--=--=--=--=--=--=--=-
	$(CXX) $(CFLAGS) $(WARNING) $(DEBUGFLAG) -c $*.cc -o $@

.c.o:
	$(rm) -f $@
	$(SHOW_COMMAND)echo --------- [$*.c] ---------------------------
	$(CC) $(CFLAGS) $(WARNING) $(DEBUGFLAG) -c $*.c -o $@
