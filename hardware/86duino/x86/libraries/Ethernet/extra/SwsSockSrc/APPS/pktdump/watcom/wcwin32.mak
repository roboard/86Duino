# Watcom makefile to build win32 app with Watcom C 1.4
#
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  01  02 Mar 06  lvr  Creation
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
TARGET = pktdump.exe


########################
# Paths

!ifdef DEBUG
OBJDIR = .\dbg
!else
OBJDIR = .\rel
!endif
.obj : $(OBJDIR)\

ROOTDIR = ..\..\..

SRC = ..;$(ROOTDIR)\libs
.c : $(SRC)
.cpp : $(SRC)

INC2 = $(ROOTDIR)\include
INCLUDE = -I$(INC2)
.h : $(INC2)

# Libraries
SOCKDIR = $(ROOTDIR)\libs\watcom


########################
# Objects to build

APPOBJS = $(OBJDIR)\pktdump.obj
APPOBJS += $(OBJDIR)\sws_dump.obj $(OBJDIR)\sws_dmpe.obj $(OBJDIR)\sws_dmpi.obj $(OBJDIR)\sws_dmpp.obj
APPOBJS += $(OBJDIR)/sws_str.obj

# Common build options
CPPFLAGS += -DSWS_USE_SOCKLIB=1 -DSWS_DLL -DWIN32


########################
# Tools

# C compiler
CC = wcc386 -bt=nt
CFLAGS = -zq
CCOUT = -fo 
CPPFLAGS += $(INCLUDE)
CSTRICT = -za

# C warnings
CFLAGS += -wx -wcd=555

# C++ compiler
CXX = wpp386 -bt=nt
CXXFLAGS = -xst

# C++ warnings
CXXFLAGS +=

# Linker
LD = wlink sys nt
LDFLAGS = OPTION QUIET, MAP
LDOUT = NAME

# Lib
AR = wlib
ARFLAGS = -q -n # -n=new

# Common build options
CFLAGS += -3r -fp3 -fpi87
CFLAGS += -oi # inline intrinsics
CFLAGS += -ox
CFLAGS += -ei -zp4

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
CFLAGS += /oh /oi /ei /5 /fpi87 /fp5
CPPFLAGS += -DNDEBUG
LDFLAGS += OPTION ELIMINATE
CFLAGS += /zm # Each function in own seg

!endif


########################
# SwsSock library

!ifndef DEBUG
SWSSOCK = sws_sock.lib
!else
SWSSOCK = sws_sckd.lib
!endif

LIBRARY = $(SOCKDIR)\$(SWSSOCK)

LIBS += LIBPATH $(SOCKDIR) LIBRARY $(SWSSOCK)
#LIBS += LIBRARY $(LIBRARY)
LIBS += LIBRARY winmm.lib,iphlpapi.lib

!ifndef DEBUG
$(LIBRARY) :
	pushd $(SOCKDIR) & $(MAKE) -f swsw32dl.mak -h -s NDEBUG=1 clean all & popd

!else
$(LIBRARY) :
	pushd $(SOCKDIR) & $(MAKE) -f swsw32dl.mak -h -s DEBUG=1 clean all & popd

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
