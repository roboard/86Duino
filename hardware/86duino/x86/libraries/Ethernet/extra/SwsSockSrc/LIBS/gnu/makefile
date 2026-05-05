#!/usr/bin/make
# GNU makefile to build the SwsSock library
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  16 May 09  lvr  Revised to use rules.mak
#  01  11 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds & runs OK.
# Cygwin (gcc 3.4.4) builds & runs  OK

ROOT_DIR ?= ../..

# Lib name
SWSSOCKLIB := swsk
ifdef DEBUG
 SWSSOCKLIB := $(SWSSOCKLIB)d
endif
ifdef DJGPP
 SWSLIBDIR ?= $(ROOT_DIR)/LIBS/DJGPP
else
ifdef XDJGPP
 #SWSSOCKLIB := $(SWSSOCKLIB)dj
 SWSLIBDIR ?= $(ROOT_DIR)/LIBS/DJGPP
else
ifdef XMINGW32
 #SWSSOCKLIB := $(SWSSOCKLIB)w32
 SWSLIBDIR ?= $(ROOT_DIR)/LIBS/mingw
else
ifdef CYGDIR
 SWSLIBDIR ?= $(ROOT_DIR)/LIBS/cygwin
else
 SWSLIBDIR ?= $(ROOT_DIR)/LIBS/linux
endif
endif
endif
endif

SHARED ?= 0
ifneq (0,$(SHARED))
 # SwsSock as a shared library
 override CPPFLAGS += -DSWS_DLL=1
 SWSSOCK ?= 1
 SWSSOCKLIB := $(SWSSOCKLIB)s
endif

ifndef TARGET
 # Force lib build from source
 override SWSSOCK =2
 ifeq (0,$(SHARED))
  TARGET = lib$(SWSSOCKLIB).a
 else
 ifdef XMINGW32
  # Build implib & dll
  TARGET := $(SWSSOCKLIB).lib
  override CPPFLAGS += -D_USRDLL=1
 else
  TARGET = lib$(SWSSOCKLIB).so
 endif
 endif
endif

# Networking: 0= Native, 1= Swssock lib, 2= Swssock source, 3= Native + Swssock support lib
# Threading, 0= None, 1= Cooperative, 2= Native
ifdef DJGPP
 # DJGPP has no native networking
 SWSSOCK ?= 1
 ifeq (3,$(SWSSOCK))
  SWSSOCK = 1
 endif
 # DJGPP has no native threads
 THREADS ?= 1
else
ifdef XDJGPP
 # DJGPP has no native networking
 SWSSOCK ?= 1
 ifeq (3,$(SWSSOCK))
  SWSSOCK = 1
 endif
 # DJGPP has no native threads
 THREADS ?= 1
else
ifdef CYGDIR
 SWSSOCK ?= 0
 THREADS ?= 2
else
ifdef XMINGW32
 SWSSOCK ?= 0
 THREADS ?= 2
else
 # Default to native networking & threads
 SWSSOCK ?= 0
 THREADS ?= 2
endif
endif
endif
endif

ifeq (0,$(SWSSOCK))
 # Native networking
 ifdef XMINGW32
  LIBS += -lws2_32 -liphlpapi
 endif

else
ifeq (1,$(SWSSOCK))
 # SwsSock networking, link with lib
 override CPPFLAGS += -DSWS_USE_SOCKLIB=1
 LIBS += -l$(SWSSOCKLIB)
 LIBPATH += $(SWSLIBDIR)

else
ifeq (2,$(SWSSOCK))
 # SwsSock networking, built from sources
 override CPPFLAGS += -DSWS_USE_SOCKLIB=1

 # Platform dependent
 ifdef DJGPP
  SOURCES += djgpp packet pccomms
  NOTANSISOURCES += djgpp packet pccomms sws_dump
  SRCPATH += $(ROOT_DIR)/SRC/DJGPP
  INCPATH += . $(ROOT_DIR)/LIBS/DJGPP $(ROOT_DIR)/SRC/DJGPP
 else
 ifdef XDJGPP
  SOURCES += djgpp packet pccomms
  NOTANSISOURCES += djgpp packet pccomms sws_dump
  SRCPATH += $(ROOT_DIR)/SRC/DJGPP
  INCPATH += $(ROOT_DIR)/LIBS/DJGPP $(ROOT_DIR)/SRC/DJGPP
 else
 ifdef XMINGW32
  SOURCES += win32 packet32 raw32 w32comms ws2
  SRCPATH += $(ROOT_DIR)/SRC/WIN32
  INCPATH += $(ROOT_DIR)/LIBS/WIN32 $(ROOT_DIR)/SRC/WIN32
  ifndef DEBUG
   OPTIMISE = -O1 # BUG 2009/06/23: gcc v4.2.1 -O2 causes data corruption
  endif
 else
 ifdef CYGDIR
  SOURCES += posix
  BSDSOURCES += posix
  SRCPATH += $(ROOT_DIR)/SRC/posix
  INCPATH += . $(ROOT_DIR)/LIBS/cygwin $(ROOT_DIR)/SRC/posix
  PSXCOMMS ?= 1
  ifneq (0,$(PSXCOMMS))
   SOURCES += psxcomms
   BSDSOURCES += psxcomms
   NOTANSISOURCES += psxcomms
  endif
 else
  SOURCES += posix
  BSDSOURCES += posix
  NOTANSISOURCES += posix
  SRCPATH += $(ROOT_DIR)/SRC/posix
  INCPATH += . $(ROOT_DIR)/LIBS/linux $(ROOT_DIR)/SRC/posix
  PACKETX ?= 1
  ifneq (0,$(PACKETX))
   SOURCES += packetx
   BSDSOURCES += packetx
   NOTANSISOURCES += packetx
  endif
  PSXCOMMS ?= 1
  ifneq (0,$(PSXCOMMS))
   SOURCES += psxcomms
   BSDSOURCES += psxcomms
   NOTANSISOURCES += psxcomms
  endif
 endif
 endif
 endif
 endif

 # Platform independent
 SRCPATH += $(ROOT_DIR)/SRC
 INCPATH += $(ROOT_DIR)/SRC

 # Common
 SOURCES += startup sockets sws_cfg gef buffer messageq timer deque sws_str

 # Protocols
 SOURCES +=  ip icmp igmp tcp udp rawip

 # Services
 SOURCES +=  dhcp sws_dns

 # Interfaces
 SOURCES += loopback ethernet p2p slip ppp sws_md5

 # Test interfaces
 SOURCES += fakenic fakep2p

 # Threading model.  0= none, 1= longjmp, 2= native/pthread
 override CPPFLAGS += -DOPT_THREADS=$(THREADS)
 ifneq (0,$(THREADS))
  SOURCES += thread
 endif

 # Packet dumping
 SOURCES += sws_dump sws_dmpi sws_dmpe sws_dmpp

else
ifeq (3,$(SWSSOCK))
 # Native networking but link with SwsSock lib
 LIBS += -l$(SWSSOCKLIB)
 LIBPATH += $(SWSLIBDIR)
else
 $(error Invalid SWSSOCK=$(SWSSOCK). Expected 0..3)
endif
endif
endif
endif

# SwsSock public interface
INCPATH += $(ROOT_DIR)/INCLUDE
SRCPATH += $(ROOT_DIR)/LIBS
ifneq (0,$(SWSSOCK))
 # SwsSock support libs
 ifdef DJGPP
 else
 ifdef XDJGPP
 else
 ifdef XMINGW32
  LIBS += -lwinmm -liphlpapi
  ifeq (3,$(SWSSOCK))
   LIBS += -lws2_32
  endif
 else
 ifdef CYGDIR
  ifeq (2,$(THREADS))
   LIBS += -lpthread
   override CPPFLAGS += -D_REENTRANT
  endif
 else
  ifeq (2,$(THREADS))
   CFLAGS += -pthread
   LDFLAGS += -pthread
   override CPPFLAGS += -D_REENTRANT
  endif
 endif
 endif
 endif
 endif
endif

include  $(ROOT_DIR)/LIBS/gnu/rules.mak

# Build the lib if it's required & doesn't exist
ifneq (0,$(SWSSOCK))
ifneq (2,$(SWSSOCK))
ifdef DJGPP
$(TARGET) : -l$(SWSSOCKLIB)
else
$(TARGET) : | -l$(SWSSOCKLIB)
endif
endif
endif

# Build options
ifdef PACKETX
$(call obj-var,posix,CPPFLAGS += -DOPT_PACKETX=$(PACKETX))
endif
ifdef PSXCOMMS
$(call obj-var,posix,CPPFLAGS += -DOPT_POSIXCOMMS=$(PSXCOMMS))
endif
ifeq (2,$(THREADS))
$(call obj-var,posix,CPPFLAGS += -D_XOPEN_SOURCE=500) # pthreads
endif

ifdef DEBUG
$(call obj-var,startup,CPPFLAGS += -DOPT_DUMP=1)
endif

# Quieten some warnings
$(call obj-var,sws_dns,WARN += -Wno-strict-aliasing)
$(call obj-var,sockets,WARN += -Wno-strict-aliasing -Wno-cast-qual)
$(call obj-var,udp,WARN += -Wno-cast-qual)
$(call obj-var,rawip,WARN += -Wno-cast-qual)
$(call obj-var,ethernet,WARN += -Wno-cast-qual)
$(call obj-var,p2p,WARN += -Wno-strict-aliasing -Wno-cast-qual)
$(call obj-var,packetx,WARN += -Wno-multichar)
$(call obj-var,psxcomms,WARN += -Wno-multichar)
ifdef XMINGW32
 $(call obj-var,win32,WARN += -Wno-multichar -Wno-cast-qual -Wno-strict-aliasing)
 $(call obj-var,win32,CWARN += -Wno-bad-function-cast -Wno-missing-prototypes -Wno-missing-declarations)
 $(call obj-var,packet32,WARN += -Wno-multichar)
 $(call obj-var,w32comms,WARN += -Wno-multichar)
 $(call obj-var,w32comms,CWARN += -Wno-bad-function-cast)
 $(call obj-var,ws2,WARN += -Wno-cast-qual)
 $(call obj-var,ws2,CWARN += -Wno-bad-function-cast -Wno-missing-prototypes -Wno-missing-declarations)
endif
ifdef CYGDIR
 WARN += -Wno-format
endif

# How to build the library
-l$(SWSSOCKLIB) :
	@$(MAKE) -s -C $(SWSLIBDIR) all tidy

.PHONY : clean-lib
clean-lib :
	@$(MAKE) -s -C $(SWSLIBDIR) clean

ifdef XMINGW32
 ifneq (0,$(SHARED))
  vpath %.def $(SWSLIBDIR)
  $(SWSSOCKLIB).lib : $(SWSSOCKLIB).def
  comma := ,
  $(call target-var,$(TARGET),LDFLAGS += -Wl$(comma)--enable-stdcall-fixup)
 endif

 .PHONY : clean-dll
 clean-dll :
	@-$(call rm,$(SWSSOCKLIB).dll)
	@-$(call rm,$(SWSSOCKLIB).lib)

 clean : clean-dll
endif
