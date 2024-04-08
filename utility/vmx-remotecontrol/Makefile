MOD=libvmx_remotectrl.so
target=vmx_remotectrl_test
target_srcs=vmx_remotectrl.c

CFLAGS += -Wall -g -fPIC 
CFLAGS += -I.
LOCAL_CFLAGS += $(DEFINES)
LOCAL_CFLAGS += -fPIC
#LOCAL_CFLAGS += -mfpu=neon

GENERIC_LINUX_PATH = ${SDKTARGETSYSROOT}/
LIBNAME=$(GENERIC_LINUX_PATH)/usr/lib64/libvmx_remotectrl.so

INCLUDES = -I${SDKTARGETSYSROOT}/../system/usr/include/ -I./
LIB_INCLUDES =-L${SDKTARGETSYSROOT}/../system/usr/lib64
LIB_INCLUDES +=-L${SDKTARGETSYSROOT}/usr/lib64/
LIBS += -pthread
LIBS += -L$(GENERIC_LINUX_PATH)/usr/lib64/ -lvmx_remotectrl

LOCAL_WHOLE_LIBRARIES = \
-Wl,--no-whole-archive \

LDFLAGS += ${LIB_INCLUDES}
CFLAGS += $(INCLUDES) $(DEFINES) $(LOCAL_CFLAGS) ${LIB_INCLUDES}
target_objs = $(patsubst %.c, %.o, $(target_srcs))

all: $(LIBNAME) $(target)

$(LIBNAME): $(target_objs)
	rm -f $@
	$(CXX) -o $@ $(target_objs) -shared
	#$(CXX) -shared -Wl,-soname=$(MOD) -o $(target_objs) $^

$(target): $(LIBNAME)
	@echo "[LD] $@"
	$(CC) $(CFLAGS) -c test_vmx_remotecontrol.c -o test_vmx_remotecontrol.o
	$(CC) $(CFLAGS) -o $(target) test_vmx_remotecontrol.o -L${SDKTARGETSYSROOT}/usr/lib64  $(LIBS) $(LOCAL_WHOLE_LIBRARIES)

clean:
	@echo "[CLEAN]"
	echo "SDKTARGETSYSROOT"
	echo ${SDKTARGETSYSROOT}
	rm -f *.o $(target) $(LIBNAME)

install:
	mkdir -p $(DESTDIR)/usr/bin/
	cp $(target) $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/lib64/
	cp $(LIBNAME) $(DESTDIR)/usr/lib64/
	mkdir -p $(DESTDIR)/usr/include/
	cp vmx_remotectrl.h $(DESTDIR)/usr/include/

%.o: %.c
	@echo "[CC] $<"
	echo "SDKTARGETSYSROOT"
	echo ${SDKTARGETSYSROOT}
	$(CC) $(CFLAGS) -c $< -o $@
