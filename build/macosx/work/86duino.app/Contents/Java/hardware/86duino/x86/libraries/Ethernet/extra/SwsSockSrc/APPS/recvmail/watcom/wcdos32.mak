# Watcom makefile to build dos32 app with Watcom C 1.4
#
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  01  20 Mar 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Instructions
# ------------
# type make -fmakefile to use. Add -DDEBUG=1 to build at debug.

# Default to ndebug build
#
!ifndef NDEBUG
!ifndef DEBUG
NDEBUG = 1
!endif
!endif

# Executable
TARGET = recvmail.exe


########################
# Paths

!ifdef DEBUG
OBJDIR = .\dbg
!else
OBJDIR = .\rel
!endif
.obj : $(OBJDIR)\

ROOTDIR = ..\..\..

SRC = ..
SRC +=;$(ROOTDIR)\LIBS
.c : $(SRC)
.cpp : $(SRC)

INC2 = $(ROOTDIR)\include
INCLUDE = -I$(INC2)
.h : $(INC2)

# Libraries
SOCKDIR = $(ROOTDIR)\libs\watcom


########################
# Objects to build

APPOBJS = $(OBJDIR)\main.obj $(OBJDIR)\recvmail.obj
APPOBJS += $(OBJDIR)/sws_str.obj $(OBJDIR)/sws_ba64.obj $(OBJDIR)/sws_time.obj

# Common build options
CPPFLAGS += -DSWS_USE_SOCKLIB=1
CPPFLAGS += -DPACKET_DUMP=1


########################
# Tools

# C compiler
CC = wcc386 -bt=dos
CFLAGS = -zq
CCOUT = -fo 
CPPFLAGS += $(INCLUDE)
CSTRICT = -za

# C warnings
CFLAGS += -wx -wcd=555

# C++ compiler
CXX = wpp386 -bt=dos
CXXFLAGS = -xst

# C++ warnings
CXXFLAGS +=

# Linker
#LD = wlink sys stub32a
LD = wlink sys dos32a
#LD = wlink sys causeway
#LD = wlink sys dos4g
LDFLAGS = OPTION QUIET,MAP,STACK=64k
LDOUT = NAME

# Lib
AR = wlib
ARFLAGS = -q -n # -n=new

# Common build options
CFLAGS += -3r -fp3 -fpi87
CFLAGS += -oi # inline intrinsics

!ifdef DEBUG

# Debug options
CFLAGS += -d2
CXXFLAGS += -d2s
CPPFLAGS += -DDEBUG=$(DEBUG)
LDFLAGS += d all op symf

!if 0
#Force C++ compile/link
CC = $(CXX)
CFLAGS += $(CXXFLAGS)
!endif

!else

# Release options
CFLAGS += -onatx
CFLAGS += /oh /oi /ei /zp8 /5 /fpi87 /fp5
CPPFLAGS += -DNDEBUG
LDFLAGS += OPTION ELIMINATE
CFLAGS += /zm # Each function in own seg

!endif


########################
# SwsSock library

!ifndef DEBUG
SWSSOCK = swsck32.lib
!else
SWSSOCK = swsck32d.lib
!endif

LIBRARY = $(SOCKDIR)\$(SWSSOCK)

LIBS += LIBPATH $(SOCKDIR) LIBRARY $(SWSSOCK)
#LIBS += LIBRARY $(LIBRARY)

!ifndef DEBUG
$(LIBRARY) :
	pushd $(SOCKDIR) & $(MAKE) -f swsck32.mak -h -s NDEBUG=1 clean all & popd

!else
$(LIBRARY) :
	pushd $(SOCKDIR) & $(MAKE) -f swsck32.mak -h -s DEBUG=1 clean all & popd

!endif

$(TARGET) : $(LIBRARY)


########################
# Implicit rules

#.SUFFIXES :
#.SUFFIXES : .c .cpp .asm .h .obj .lib

.c.obj: .AUTODEPEND
	$(CC) $(CSTRICT) $(CFLAGS) $(CPPFLAGS) $(CCOUT)$@ $<
        wdis -s -l $@

.cpp.obj: .AUTODEPEND
	$(CXX) $(CSTRICT) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(CCOUT)$@ $<
        wdis -s -l $@


########################
# Explicit rules for building objects go here.

all : $(TARGET)

$(OBJDIR) : .SYMBOLIC
	-mkdir $(OBJDIR)

$(TARGET) : $(OBJDIR) $(APPOBJS)
	$(LD) $(LDFLAGS) $(LDOUT) $@ FILE $(APPOBJS: =,) $(LIBS)

clean : .SYMBOLIC
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.lst del $(OBJDIR)\*.lst
	-if exist $(OBJDIR) rmdir $(OBJDIR)
	-if exist $(TARGET) del $(TARGET)

# End of file
