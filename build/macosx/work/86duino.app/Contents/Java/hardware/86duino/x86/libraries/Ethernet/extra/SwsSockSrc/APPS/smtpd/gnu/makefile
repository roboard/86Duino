#!/usr/bin/make
# build smtpd
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

TARGET = smtpd
SOURCES := $(TARGET) main sws_ba64 sws_time
BSDSOURCES := main $(TARGET)
POSIXSOURCES := $(TARGET)
NOTANSISOURCES := $(TARGET)
SRCPATH = ..
INCPATH = ..

SMTP_FORWARD ?= 1
ifneq (0,$(SMTP_FORWARD))
SOURCES += sendmail
BSDSOURCES += sendmail
SRCPATH += ../../sendmail
INCPATH += ../../sendmail
endif

UPNP ?= 1
ifneq (0,$(UPNP))
SOURCES += sws_upnp
BSDSOURCES += sws_upnp
endif

ifndef SWSSOCK
 SOURCES += sws_cfg sws_dns sws_str
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,main,CPPFLAGS += -DPACKET_DUMP=1)
$(call obj-var,smtpd,CPPFLAGS += -DOPT_FORWARD=$(SMTP_FORWARD) -DOPT_UPNP=$(UPNP))
$(call obj-var,smtpd,WARN += -Wno-cast-qual)
ifdef CONNECTIONS
$(call obj-var,smtpd,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif
