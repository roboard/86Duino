#!/usr/bin/make
# build upnp
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  01  30 Aug 09  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = upnp
SOURCES := $(TARGET) sws_upnp
BSDSOURCES := $(TARGET) sws_upnp
SRCPATH = ..
INCPATH = ..

ifndef SWSSOCK
 SOURCES += sws_str
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,$(TARGET),CPPFLAGS += -DPACKET_DUMP=1)
