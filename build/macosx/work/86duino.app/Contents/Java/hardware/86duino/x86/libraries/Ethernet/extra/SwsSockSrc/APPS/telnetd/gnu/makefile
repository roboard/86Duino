#!/usr/bin/make
# build telnetd
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  17 May 09  lvr  Revised to use included makefile
#  01  10 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = telnetd
SOURCES := $(TARGET) main
BSDSOURCES := main $(TARGET)
POSIXSOURCES := $(TARGET)
NOTANSISOURCES := $(TARGET)
SRCPATH = ..
INCPATH = ..

UPNP ?= 1
ifneq (0,$(UPNP))
 SOURCES += sws_upnp
 BSDSOURCES += sws_upnp
 ifndef SWSSOCK
  SOURCES += sws_str
 endif
endif

ifndef SWSSOCK
 SOURCES += sws_cfg
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,telnetd,WARN += -Wno-cast-qual)
#TELNETD_EXT: 0= none, 1= use system(), 2= use POSIX popen
EXTERN ?= 2
$(call obj-var,telnetd,CPPFLAGS += -DTELNETD_EXT=$(EXTERN) -DOPT_UPNP=$(UPNP))
ifdef CONNECTIONS
$(call obj-var,telnetd,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif
