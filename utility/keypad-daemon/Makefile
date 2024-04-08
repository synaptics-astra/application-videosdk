ROOT_FS=$(DESTDIR)

rm=/bin/rm -f
cp=/bin/cp -f

MACH_680=vs680
MACH_640=vs640
MACH_120=dvf120

ifeq ($(CURR_MACH), $(MACH_680))
LOCAL_CFLAGS += -DCURR_MACH_VS680
else
ifeq ($(CURR_MACH), $(MACH_640))
LOCAL_CFLAGS += -DCURR_MACH_VS640
else
ifeq ($(CURR_MACH), $(MACH_120))
LOCAL_CFLAGS += -DCURR_MACH_DVF120
endif
endif
endif

PROGNAME=keypad-daemon

LOCAL_SRC_FILES :=	main.c \
					keypad.c

LOCAL_C_INCLUDES += \
					. \
					$(SDKTARGETSYSROOT)/usr/include \

LIBS = -pthread

LOCAL_CFLAGS += -fPIC -Wall -g -fPIC -I.

OBJ_FILES = $(addsuffix .o, $(basename $(LOCAL_SRC_FILES)))
INCLUDES = $(addprefix -I, $(LOCAL_C_INCLUDES) )


LOCAL_WHOLE_LIBRARIES = \
-Wl,--no-whole-archive \

LIB_INCLUDES +=-L${SDKTARGETSYSROOT}/usr/lib64/:${SDKTARGETSYSROOT}/lib64/

CFLAGS += $(INCLUDES) $(DEFINES) $(LOCAL_CFLAGS) $(MYDEFS) -pthread

$(PROGNAME) : $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJ_FILES) -L${LIB_INCLUDES} $(LIBS) $(LOCAL_WHOLE_LIBRARIES)
	echo "keypad-daemon Compiled successfully..!!"


all: $(PROGNAME)

install:
	mkdir -p $(ROOT_FS)/usr/bin
	$(SHOW_COMMAND)$(cp) $(PROGNAME) $(ROOT_FS)/usr/bin
	echo "Build successfully..!!"

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