#!/usr/bin/make
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  03  14 May 09  lvr  Revised to use makefile
#  02  21 May 05  lvr  Builds libssock.a using external makefile
#  01  04 May 01  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = echos
SOURCES := echo echod
BSDSOURCES := echo echod
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

$(call obj-var,echo,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,echod,CPPFLAGS += -DOPT_UPNP=$(UPNP))
ifdef CONNECTIONS
$(call obj-var,echod,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif
