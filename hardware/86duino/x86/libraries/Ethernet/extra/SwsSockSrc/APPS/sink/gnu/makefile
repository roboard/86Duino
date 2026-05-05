#!/usr/bin/make
# build sink
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

TARGET = sink
SOURCES = echo echod
BSDSOURCES := $(SOURCES)
SRCPATH = ../../echos
INCPATH = ../../echos
CPPFLAGS += -DOPT_ECHO=0

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

$(call obj-var,echo,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,echod,CPPFLAGS += -DOPT_UPNP=$(UPNP))
