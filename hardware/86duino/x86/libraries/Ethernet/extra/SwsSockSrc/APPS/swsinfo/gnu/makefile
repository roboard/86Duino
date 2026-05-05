#!/usr/bin/make
# build swsinfo
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  01  19 Jul 10  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = swsinfo
SOURCES := $(TARGET)
BSDSOURCES := $(TARGET)
SRCPATH = ..
INCPATH = . ..

# Swssock
override SWSSOCK = 1
#override SHARED = 0
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

all : suid

