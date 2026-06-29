#!/usr/bin/make
# build popd
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use included makefile
#  01  11 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = popd
SOURCES := $(TARGET) main popmsg sws_ba64
BSDSOURCES := main popd
POSIXSOURCES := popmsg
NOTANSISOURCES := popmsg
SRCPATH = ..
INCPATH = ..

UPNP ?= 1
ifneq (0,$(UPNP))
 SOURCES += sws_upnp
 BSDSOURCES += sws_upnp
endif

CRAM_MD5 ?= 1

ifndef SWSSOCK
 SOURCES += sws_cfg sws_str
 ifneq (0,$(CRAM_MD5))
  SOURCES += sws_md5
 endif
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,popd,CPPFLAGS += -DOPT_UPNP=$(UPNP) -DOPT_CRAM_MD5=$(CRAM_MD5))
$(call obj-var,popd,WARN += -Wno-cast-qual)
$(call obj-var,popmsg,WARN += -Wno-cast-qual)
ifdef CONNECTIONS
$(call obj-var,popd,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif
