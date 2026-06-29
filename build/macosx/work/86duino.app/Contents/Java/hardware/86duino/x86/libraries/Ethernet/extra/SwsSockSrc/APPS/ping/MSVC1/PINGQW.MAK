# Microsoft Visual C++ generated build script - Do not modify

PROJ = PINGQW
DEBUG = 0
PROGTYPE = 3
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\SWSSOCK\APPS\PING\MSVC1\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = PING.C      
FIRSTCPP =             
RC = rc
CFLAGS_D_WTTY = /nologo /Gs /G2 /Gy /Mq /W4 /Gf /Zi /AL /Od /D "OPT_PING_RAWIP"="0" /D "_DEBUG" /D "WIN16" /I "." /I ".." /I "..\..\..\src\dos16" /I "..\..\..\src" /I "..\..\..\include" /FR /Fd"PINGQW.PDB"
CFLAGS_R_WTTY = /nologo /f- /Gs /G2 /Gy /Mq /W4 /Gf /AL /O1 /Oi /Ox /Ob1 /D "OPT_PING_RAWIP"="0" /D "NDEBUG" /D "WIN16" /I "." /I ".." /I "..\..\..\src\dos16" /I "..\..\..\src" /I "..\..\..\include" 
LFLAGS_D_WTTY = /NOLOGO /NOD /PACKC:57344 /ALIGN:16 /ONERROR:NOEXE /CO /MAP  
LFLAGS_R_WTTY = /NOLOGO /NOD /PACKC:57344 /ALIGN:16 /ONERROR:NOEXE /MAP  
LIBS_D_WTTY = oldnames libw llibcewq winsock mmsystem.lib 
LIBS_R_WTTY = oldnames libw llibcewq winsock mmsystem.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WTTY)
LFLAGS = $(LFLAGS_D_WTTY)
LIBS = $(LIBS_D_WTTY)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
DEFFILE=C:\MSVC\BIN\CL.DEF
!else
CFLAGS = $(CFLAGS_R_WTTY)
LFLAGS = $(LFLAGS_R_WTTY)
LIBS = $(LIBS_R_WTTY)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
DEFFILE=C:\MSVC\BIN\CL.DEF
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = PING.SBR \
		BUFFER.SBR \
		DEQUE.SBR \
		ETHERNET.SBR \
		GEF.SBR \
		IP.SBR \
		MESSAGEQ.SBR \
		SOCKETS.SBR \
		TIMER.SBR \
		DOS16.SBR \
		PACKET.SBR \
		SWS_CFG.SBR \
		SWS_DMPE.SBR \
		SWS_DMPI.SBR \
		SWS_DMPP.SBR \
		SWS_DUMP.SBR \
		DHCP.SBR \
		FAKENIC.SBR \
		FAKEP2P.SBR \
		ICMP.SBR \
		IGMP.SBR \
		LOOPBACK.SBR \
		P2P.SBR \
		PPP.SBR \
		RAWIP.SBR \
		SLIP.SBR \
		STARTUP.SBR \
		TCP.SBR \
		THREAD.SBR \
		UDP.SBR \
		PCCOMMS.SBR \
		SWS_MD5.SBR \
		SWS_STR.SBR


BUFFER_DEP = c:\work\swssock\src\platform.h


BUFFER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


DEQUE_DEP = c:\work\swssock\src\deque.h \
	c:\work\swssock\src\debug.h


ETHERNET_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


GEF_DEP = c:\work\swssock\src\debug.h


IP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\loopback.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\debug.h


MESSAGEQ_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SOCKETS_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


TIMER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


DHCP_DEP = c:\work\swssock\src\debug.h


DHCP_DEP = c:\work\swssock\src\platform.h


FAKENIC_DEP = c:\work\swssock\src\fakenic.h


FAKENIC_DEP = c:\work\swssock\src\debug.h


FAKEP2P_DEP = c:\work\swssock\src\fakep2p.h


FAKEP2P_DEP = c:\work\swssock\src\p2p.h


FAKEP2P_DEP = c:\work\swssock\src\platform.h


FAKEP2P_DEP = c:\work\swssock\src\debug.h


ICMP_DEP = c:\work\swssock\src\ip.h


ICMP_DEP = c:\work\swssock\src\platform.h


ICMP_DEP = c:\work\swssock\src\debug.h


IGMP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


LOOPBACK_DEP = c:\work\swssock\src\loopback.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


P2P_DEP = c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\slip.h \
	c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


PPP_DEP = c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


RAWIP_DEP = c:\work\swssock\src\rawip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SLIP_DEP = c:\work\swssock\src\slip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


STARTUP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\rawip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\thread.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


TCP_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


THREAD_DEP = c:\work\swssock\src\thread.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


UDP_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


all:	$(PROJ).EXE

PING.OBJ:	..\PING.C $(PING_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\PING.C

BUFFER.OBJ:	..\..\..\SRC\BUFFER.C $(BUFFER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\BUFFER.C

DEQUE.OBJ:	..\..\..\SRC\DEQUE.C $(DEQUE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DEQUE.C

ETHERNET.OBJ:	..\..\..\SRC\ETHERNET.C $(ETHERNET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\ETHERNET.C

GEF.OBJ:	..\..\..\SRC\GEF.C $(GEF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\GEF.C

IP.OBJ:	..\..\..\SRC\IP.C $(IP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\IP.C

MESSAGEQ.OBJ:	..\..\..\SRC\MESSAGEQ.C $(MESSAGEQ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\MESSAGEQ.C

SOCKETS.OBJ:	..\..\..\SRC\SOCKETS.C $(SOCKETS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\SOCKETS.C

TIMER.OBJ:	..\..\..\SRC\TIMER.C $(TIMER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\TIMER.C

DOS16.OBJ:	..\..\..\SRC\DOS16\DOS16.C $(DOS16_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\DOS16.C

PACKET.OBJ:	..\..\..\SRC\DOS16\PACKET.C $(PACKET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\PACKET.C

SWS_CFG.OBJ:	..\..\..\LIBS\SWS_CFG.C $(SWS_CFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_CFG.C

SWS_DMPE.OBJ:	..\..\..\LIBS\SWS_DMPE.C $(SWS_DMPE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_DMPE.C

SWS_DMPI.OBJ:	..\..\..\LIBS\SWS_DMPI.C $(SWS_DMPI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_DMPI.C

SWS_DMPP.OBJ:	..\..\..\LIBS\SWS_DMPP.C $(SWS_DMPP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_DMPP.C

SWS_DUMP.OBJ:	..\..\..\LIBS\SWS_DUMP.C $(SWS_DUMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_DUMP.C

DHCP.OBJ:	..\..\..\SRC\DHCP.C $(DHCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DHCP.C

FAKENIC.OBJ:	..\..\..\SRC\FAKENIC.C $(FAKENIC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\FAKENIC.C

FAKEP2P.OBJ:	..\..\..\SRC\FAKEP2P.C $(FAKEP2P_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\FAKEP2P.C

ICMP.OBJ:	..\..\..\SRC\ICMP.C $(ICMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\ICMP.C

IGMP.OBJ:	..\..\..\SRC\IGMP.C $(IGMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\IGMP.C

LOOPBACK.OBJ:	..\..\..\SRC\LOOPBACK.C $(LOOPBACK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\LOOPBACK.C

P2P.OBJ:	..\..\..\SRC\P2P.C $(P2P_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\P2P.C

PPP.OBJ:	..\..\..\SRC\PPP.C $(PPP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\PPP.C

RAWIP.OBJ:	..\..\..\SRC\RAWIP.C $(RAWIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\RAWIP.C

SLIP.OBJ:	..\..\..\SRC\SLIP.C $(SLIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\SLIP.C

STARTUP.OBJ:	..\..\..\SRC\STARTUP.C $(STARTUP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\STARTUP.C

TCP.OBJ:	..\..\..\SRC\TCP.C $(TCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\TCP.C

THREAD.OBJ:	..\..\..\SRC\THREAD.C $(THREAD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\THREAD.C

UDP.OBJ:	..\..\..\SRC\UDP.C $(UDP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\UDP.C

PCCOMMS.OBJ:	..\..\..\SRC\DOS16\PCCOMMS.C $(PCCOMMS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\PCCOMMS.C

SWS_MD5.OBJ:	..\..\..\LIBS\SWS_MD5.C $(SWS_MD5_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_MD5.C

SWS_STR.OBJ:	..\..\..\LIBS\SWS_STR.C $(SWS_STR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_STR.C


$(PROJ).EXE::	PING.OBJ BUFFER.OBJ DEQUE.OBJ ETHERNET.OBJ GEF.OBJ IP.OBJ MESSAGEQ.OBJ \
	SOCKETS.OBJ TIMER.OBJ DOS16.OBJ PACKET.OBJ SWS_CFG.OBJ SWS_DMPE.OBJ SWS_DMPI.OBJ \
	SWS_DMPP.OBJ SWS_DUMP.OBJ DHCP.OBJ FAKENIC.OBJ FAKEP2P.OBJ ICMP.OBJ IGMP.OBJ LOOPBACK.OBJ \
	P2P.OBJ PPP.OBJ RAWIP.OBJ SLIP.OBJ STARTUP.OBJ TCP.OBJ THREAD.OBJ UDP.OBJ PCCOMMS.OBJ \
	SWS_MD5.OBJ SWS_STR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
PING.OBJ +
BUFFER.OBJ +
DEQUE.OBJ +
ETHERNET.OBJ +
GEF.OBJ +
IP.OBJ +
MESSAGEQ.OBJ +
SOCKETS.OBJ +
TIMER.OBJ +
DOS16.OBJ +
PACKET.OBJ +
SWS_CFG.OBJ +
SWS_DMPE.OBJ +
SWS_DMPI.OBJ +
SWS_DMPP.OBJ +
SWS_DUMP.OBJ +
DHCP.OBJ +
FAKENIC.OBJ +
FAKEP2P.OBJ +
ICMP.OBJ +
IGMP.OBJ +
LOOPBACK.OBJ +
P2P.OBJ +
PPP.OBJ +
RAWIP.OBJ +
SLIP.OBJ +
STARTUP.OBJ +
TCP.OBJ +
THREAD.OBJ +
UDP.OBJ +
PCCOMMS.OBJ +
SWS_MD5.OBJ +
SWS_STR.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
C:\MSVC\LIB\+
C:\MSVC\MFC\LIB\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@


run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
