#!/usr/bin/make
# build httpd
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  03  24 Oct 09  lvr  Added httpprxy
#  02  14 May 09  lvr  Revised to use makefile
#  01  10 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = httpd
SOURCES := main httpd httpfile sws_str
BSDSOURCES := main httpd
POSIXSOURCES := httpfile
NOTANSISOURCES := httpfile
SRCPATH = ..
INCPATH = ..

# Minimal build
TINY ?= 0
ifneq (0,$(TINY))
UPNP = 0
PROXY = 0
CGI = 0
LFG = 0
NOV4L = 1
else
 SOURCES += sws_ba64 sws_time
endif

# UPnP port mapping
UPNP ?= 1
ifneq (0,$(UPNP))
SOURCES += sws_upnp
BSDSOURCES += sws_upnp
endif

# Proxy server
PROXY ?= 1
ifneq (0,$(PROXY))
SOURCES += httpprxy
BSDSOURCES += httpprxy
endif

# CGI extensions
CGI ?= 1
ifneq (0,$(CGI))
SOURCES += httpcgi
BSDSOURCES += httpcgi
POSIXSOURCES += httpcgi
NOTANSISOURCES += httpcgi
endif

# ini file configuration
ifndef SWSSOCK
 SOURCES += sws_cfg
endif

# webcam
ifdef XMINGW32
 LFG ?= 0
 ifneq (0,$(LFG))
 CPPFLAGS += -DOPT_HTTP_CAMERA=1 -D_LFG_WIN32 -D_TMG_WIN32
 SOURCES += httplfg lfg_tmg
 ifeq (cxx,$(DEBUG))
  NOTANSISOURCES += httplfg lfg_tmg
 endif
 LFGROOT = ../LFGSDK
 SRCPATH += $(LFGROOT)/SRC
 INCPATH += $(LFGROOT)/INC
 LIBPATH += $(LFGROOT)/LIB
 LIBS += -lLfgw32
 ifeq (0,0)
  LIBS += -lLfgTmg32
 else
  LIBS += -lTMGWIN32
 endif
 # TODO: Weak import / delay loaded
 endif
else
ifdef CYGDIR
 # TODO
else
ifdef DJGPP
 LFG ?= 0
 ifneq (0,$(LFG))
 CPPFLAGS += -DOPT_HTTP_CAMERA=1 -D_LFG_DOS32 -D_CDA_DOS32 -D_TMG_DOS32
 SOURCES += httplfg lfg_tmg
 ifeq (cxx,$(DEBUG))
   NOTANSISOURCES += httplfg lfg_tmg
 endif
 LFGROOT = ../LFGDJG
 SRCPATH += $(LFGROOT)/SRC
 INCPATH += $(LFGROOT)/INC
 LIBPATH += $(LFGROOT)/LIB
 LIBS += -llfg -lcda -ltmg
 endif
else
ifdef XDJGPP
 LFG ?= 0
 ifneq (0,$(LFG))
 CPPFLAGS += -DOPT_HTTP_CAMERA=1 -D_LFG_DOS32 -D_CDA_DOS32 -D_TMG_DOS32
 SOURCES += httplfg lfg_tmg
 ifeq (cxx,$(DEBUG))
   NOTANSISOURCES += httplfg lfg_tmg
 endif
 LFGROOT = ../LFGDJG
 SRCPATH += $(LFGROOT)/SRC
 INCPATH += $(LFGROOT)/INC
 LIBPATH += $(LFGROOT)/LIB
 LIBS += -llfg -lcda -ltmg
 endif
else
 ifndef NOV4L
 CPPFLAGS += -DOPT_HTTP_V4L=1
 SOURCES += httpv4l
 BSDSOURCES += httpv4l
 LIBS += -ljpeg
 endif
endif
endif
endif
endif

# Swssock
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

# Build options
$(call obj-var,main,CPPFLAGS +=-DPACKET_DUMP=1 -DOPT_HTTP_PROXY=$(PROXY) -DOPT_HTTP_CGI=$(CGI))
$(call obj-var,httpd,CPPFLAGS +=-DOPT_HTTP_TINY=$(TINY) -DOPT_UPNP=$(UPNP))
$(call obj-var,httpfile,CPPFLAGS +=-DOPT_HTTP_TINY=$(TINY))
ifdef SAVEBODY
$(call obj-var,httpd,CPPFLAGS += -DOPT_SAVE_BODY=$(SAVEBODY))
endif
ifdef CONNECTIONS
$(call obj-var,httpd,CPPFLAGS += -DOPT_MAX_CONNECTIONS=$(CONNECTIONS))
endif

# Build warnings
$(call obj-var,httpd,WARN += -Wno-cast-qual)
$(call obj-var,lfg_tmg,WARN += -Wno-strict-aliasing)
