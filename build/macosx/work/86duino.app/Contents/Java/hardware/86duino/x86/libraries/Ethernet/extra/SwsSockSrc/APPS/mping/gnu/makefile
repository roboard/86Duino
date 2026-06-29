#!/usr/bin/make
# build mping
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use makefile
#  01  11 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = mping
SOURCES := $(TARGET)
BSDSOURCES := $(TARGET)
SRCPATH = ..
INCPATH = ..

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call target-var,$(TARGET),CPPFLAGS += -DPACKET_DUMP=1)

all : suid
