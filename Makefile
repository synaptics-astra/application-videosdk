CROSS ?= arm-dspg-linux-gnueabi-
export CROSS
DESTDIR ?=
export DESTDIR
export CFLAGS
export TOPDIR

MACH_680=vs680
MACH_640=vs640
MACH_120=dvf120

V4L2_ENABLE?=0
AMP_ENABLE?=0

all: 
	$(S)make -sC codec-demo -f Makefile all
ifeq ($(AMP_ENABLE), 1)
ifeq ($(CURR_MACH), $(MACH_680))
	$(S)make -sC utility/keypad-daemon -f Makefile
	$(S)make -sC utility/vmx-remotecontrol -f Makefile
	$(S)make -sC v4l2hdmiRxExt_test -f Makefile
endif
endif

clean: 
	$(S)make -sC codec-demo -f Makefile clean all
ifeq ($(AMP_ENABLE), 1)
ifeq ($(CURR_MACH), $(MACH_680))
	$(S)make -sC utility/keypad-daemon -f Makefile clean
	$(S)make -sC utility/vmx-remotecontrol -f Makefile clean
	$(S)make -sC v4l2hdmiRxExt_test -f Makefile clean
endif
endif

install: 
	$(S)make -sC codec-demo -f Makefile install all
ifeq ($(AMP_ENABLE), 1)
ifeq ($(CURR_MACH), $(MACH_680))
	$(S)make -sC utility/keypad-daemon -f Makefile install
	$(S)make -sC utility/vmx-remotecontrol -f Makefile install
	$(S)make -sC v4l2hdmiRxExt_test -f Makefile install
endif
endif
