target = v4l2hdmiext-test
target_srcs = v4l2hdmiExt.c v4l2hdmiExt_test.c

CFLAGS += -Wall -g -fPIC
CFLAGS += -I.
LOCAL_CFLAGS += $(DEFINES)
LOCAL_CFLAGS += -fPIC
CFLAGS += -I${TOPDIR}/application/libsynav4l2/inc/
CFLAGS += -I./../codec-demo

INCLUDES = -I${SDKTARGETSYSROOT}/../system/usr/include/
LIBS += -lsynav4l2 -lampclient -lOSAL
LIBS += -pthread
LIB_INCLUDES =-L${SDKTARGETSYSROOT}/../system/usr/lib
LIB_INCLUDES +=-L${SDKTARGETSYSROOT}/usr/lib/
LIB_INCLUDES += -L${TOP}/sysroot/linux-baseline/data/gfx_prebuilt/imagination/local/lib $(LIBS)

LDFLAGS += ${LIB_INCLUDES}
CFLAGS += $(INCLUDES) $(DEFINES) $(LOCAL_CFLAGS) ${LIB_INCLUDES}
target_objs = $(patsubst %.c, %.o, $(target_srcs))

all: $(target)

$(target): $(target_objs)
	@echo "[LD] $@"
	$(CC)  $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@echo "[CLEAN]"
	rm -f *.o $(target)

install:
	mkdir -p $(DESTDIR)/usr/bin/
	cp $(target) $(DESTDIR)/usr/bin/

%.o: %.c
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@
