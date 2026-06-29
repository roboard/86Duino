#!/usr/bin/make
# build pktdump
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use included makefile
#  01  11 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = pktdump
SOURCES := $(TARGET)
BSDSOURCES := $(TARGET)
SRCPATH = ..
INCPATH = . ..

# Swssock
override SWSSOCK = 2
override SHARED = 0
THREADS = 0
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,startup,CPPFLAGS += -DOPT_DUMP=1)
$(call obj-var,sws_dump,CPPFLAGS += -DDOPT_DUMP_PPP=1)
$(call obj-var,sws_dmpe,CPPFLAGS += -DDOPT_DUMP_PPP=1)

all : suid
