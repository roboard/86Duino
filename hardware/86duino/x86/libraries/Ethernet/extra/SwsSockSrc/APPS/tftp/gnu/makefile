#!/usr/bin/make
# build tftp
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  17 May 09  lvr  Revised to use included makefile
#  01  11 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = tftp
SOURCES := $(TARGET) main
BSDSOURCES := $(TARGET) main
SRCPATH = ..
INCPATH = ..

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,tftp,WARN += -Wno-strict-aliasing)
