#!/usr/bin/make
# build ftpd
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  17 May 09  lvr  Revised to use makefile
#  01  10 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = ftpd
SOURCES = ftpd main
BSDSOURCES = ftpd main
POSIXSOURCES = ftpd
NOTANSISOURCES = ftpd
SRCPATH = ..
INCPATH = ..

UPNP ?= 1
ifneq (0,$(UPNP))
SOURCES += sws_upnp
BSDSOURCES += sws_upnp
endif

ifndef SWSSOCK
 SOURCES += sws_cfg sws_str
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,ftpd,CPPFLAGS += -DOPT_UPNP=$(UPNP))
$(call obj-var,ftpd,WARN += -Wno-shadow -Wno-cast-qual)
ifdef CONNECTIONS
$(call obj-var,ftpd,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif
