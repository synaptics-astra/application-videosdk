CROSS ?= arm-dspg-linux-gnueabi-
export CROSS
DESTDIR ?=
export DESTDIR
export CFLAGS
export TOPDIR

MACH_DOLPHIN=dolphin
MACH_PLATYPUS=platypus
MACH_MYNA2=myna2

V4L2_ENABLE?=0

all: 
	$(S)make -sC codec-demo -f Makefile all

clean: 
	$(S)make -sC codec-demo -f Makefile clean all

install: 
	$(S)make -sC codec-demo -f Makefile install all
