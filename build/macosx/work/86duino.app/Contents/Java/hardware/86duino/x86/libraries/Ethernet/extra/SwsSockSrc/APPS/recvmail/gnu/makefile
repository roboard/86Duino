#!/usr/bin/make
# build recvmail
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use included makefile
#  01  15 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = recvmail
SOURCES := $(TARGET) main sws_ba64 sws_str sws_time
BSDSOURCES := $(TARGET) main
SRCPATH = ..
INCPATH = ..
ifndef SWSSOCK
 SOURCES += sws_cfg
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,recvmail,WARN += -Wno-float-equal -Wno-cast-qual)
