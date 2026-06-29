# Makefile to build the SwsSock library for 16-bit DOS with Watcom C 1.4
#
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  01  01 Mar 06  lvr  Creation
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

# Library base name
!ifndef DEBUG
TARGET = swsck16.lib
!else
TARGET = swsck16d.lib
!endif


########################
# Paths

!ifdef DEBUG
OBJDIR = .\dbg
!else
OBJDIR = .\rel
!endif
.obj : $(OBJDIR)\

ROOTDIR = ..\..

SRC = $(ROOTDIR)\src\dos16;$(ROOTDIR)\src;$(ROOTDIR)\libs
.c : $(SRC)
.cpp : $(SRC)

INC =.\dos16
INCLUDE += -I$(INC)
INCLUDE += -I$(ROOTDIR)\src\dos16
INCLUDE += -I$(ROOTDIR)\src
INCLUDE += -I$(ROOTDIR)\include
.h : $(INC);$(SRC)

# Libraries
LIBS =


########################
# Objects to build

C0=$(OBJDIR)\dos16.obj $(OBJDIR)\startup.obj $(OBJDIR)\sockets.obj $(OBJDIR)\sws_cfg.obj
C1=$(OBJDIR)\buffer.obj $(OBJDIR)\messageq.obj $(OBJDIR)\timer.obj $(OBJDIR)\deque.obj $(OBJDIR)\thread.obj
C2=$(OBJDIR)\ip.obj $(OBJDIR)\icmp.obj $(OBJDIR)\igmp.obj $(OBJDIR)\gef.obj
C3=$(OBJDIR)\tcp.obj $(OBJDIR)\udp.obj $(OBJDIR)\rawip.obj $(OBJDIR)\dhcp.obj $(OBJDIR)\sws_dns.obj
C4=$(OBJDIR)\loopback.obj $(OBJDIR)\ethernet.obj $(OBJDIR)\packet.obj
C5=$(OBJDIR)\p2p.obj $(OBJDIR)\slip.obj $(OBJDIR)\ppp.obj $(OBJDIR)\sws_md5.obj $(OBJDIR)\pccomms.obj
C6=$(OBJDIR)\sws_dump.obj $(OBJDIR)\sws_dmpi.obj $(OBJDIR)\sws_dmpe.obj $(OBJDIR)\sws_dmpp.obj 
OBJS=$(C0) $(C1) $(C2) $(C3) $(C4) $(C5) $(C6)

# Common build options
CPPFLAGS += -DSWS_USE_SOCKLIB=1 -D_DOS16=1


########################
# Tools

# C compiler
CC = wcc -bt=dos
CFLAGS = -zq
CCOUT = -fo 
CPPFLAGS += $(INCLUDE)
CSTRICT = -za

# C warnings
CFLAGS += -wx -wcd=555

# C++ compiler
CXX = wpp -bt=dos
CXXFLAGS = -xst

# C++ warnings
CXXFLAGS +=

# Linker
LD = wlink sys dos
LDFLAGS = OPTION QUIET, MAP
LDOUT = NAME

# Lib
AR = wlib
ARFLAGS = -q -n # -n=new

# Common build options
CFLAGS += -ml # Memory model
CFLAGS += -zu # ss != ds
#CFLAGS += -zc # consts & strings in cs
CFLAGS += -1 -fp2 -fpi87 # 186, x87 ops
CFLAGS += -oi # inline intrinsics

!ifdef DEBUG

# Debug options
CFLAGS += -d2
CFLAGS += -zt16 # Far data threshold
CXXFLAGS += -d2s
#CPPFLAGS += -DDEBUG=$(DEBUG)
LDFLAGS += d all op symf

!if 0
#Force C++ compile/link
CC = $(CXX)
CFLAGS += $(CXXFLAGS)
!endif

!else

# Release options
CFLAGS += -onasx
CFLAGS += /oh /oi
CPPFLAGS += -DNDEBUG
LDFLAGS += OPTION ELIMINATE, FARCALLS
CFLAGS += /zm # Each function in own seg

!endif


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

$(TARGET) : $(OBJDIR) $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean : .SYMBOLIC
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.lst del $(OBJDIR)\*.lst
	-if exist $(OBJDIR) rmdir $(OBJDIR)
	-if exist $(TARGET) del $(TARGET)

$(OBJDIR)\packet.obj : packet.c .AUTODEPEND
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CCOUT)$@ $<
        wdis -s -l $@

# End of file
