#!/usr/bin/make
# build ntp
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use included makefile
#  01  10 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = ntp
SOURCES := $(TARGET) main
BSDSOURCES := $(TARGET) main
POSIXSOURCES := $(TARGET) main
NOTANSISOURCES := $(TARGET) main
SRCPATH = ..
INCPATH = ..
ifndef SWSSOCK
 SOURCES += sws_cfg
endif
ifndef XDJGPP
 LIBS += -lm
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
