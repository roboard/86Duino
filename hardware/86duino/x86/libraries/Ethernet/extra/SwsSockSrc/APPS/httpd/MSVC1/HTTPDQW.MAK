# Microsoft Visual C++ generated build script - Do not modify

PROJ = HTTPDQW
DEBUG = 1
PROGTYPE = 3
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\SWSSOCK\APPS\HTTPD\MSVC1\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = HTTPD.C     
FIRSTCPP =             
RC = rc
CFLAGS_D_WTTY = /nologo /Gs /G2 /FPi87 /Gy /Mq /W4 /Gf /Zi /AL /Gt128 /Od /D "SWS_USE_SOCKLIB"="0" /D "_DEBUG" /D "WIN16" /I "." /I ".." /I "..\..\..\src\dos16" /I "..\..\..\src" /I "..\..\..\include" /FR /Fd"HTTPDQW.PDB"
CFLAGS_R_WTTY = /nologo /f- /Gs /G3 /FPi87 /Gy /Mq /W4 /Gf /AL /Gt128 /O2 /Ob1 /D "SWS_USE_SOCKLIB"="0" /D "NDEBUG" /D "WIN16" /I "." /I ".." /I "..\..\..\src\dos16" /I "..\..\..\src" /I "..\..\..\include" 
LFLAGS_D_WTTY = /NOLOGO /NOD /PACKC:57344 /FARCALL /ALIGN:16 /ONERROR:NOEXE /CO /MAP  
LFLAGS_R_WTTY = /NOLOGO /NOD /PACKC:57344 /FARCALL /ALIGN:16 /ONERROR:NOEXE /MAP  
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
SBRS = HTTPD.SBR \
		HTTPFILE.SBR \
		SOCKETS.SBR \
		BUFFER.SBR \
		ETHERNET.SBR \
		ICMP.SBR \
		IP.SBR \
		LOOPBACK.SBR \
		MESSAGEQ.SBR \
		RAWIP.SBR \
		STARTUP.SBR \
		TIMER.SBR \
		UDP.SBR \
		DOS16.SBR \
		TCP.SBR \
		MAIN.SBR \
		PACKET.SBR \
		DEQUE.SBR \
		DHCP.SBR \
		IGMP.SBR \
		SLIP.SBR \
		THREAD.SBR \
		SWS_CFG.SBR \
		P2P.SBR \
		PPP.SBR \
		PCCOMMS.SBR \
		GEF.SBR \
		SWS_MD5.SBR \
		SWS_BA64.SBR \
		SWS_STR.SBR \
		SWS_TIME.SBR


HTTPD_DEP = c:\work\swssock\apps\httpd\httpd.h


HTTPFILE_DEP = c:\work\swssock\apps\httpd\httpfile.h \
	c:\work\swssock\apps\httpd\httpd.h


SOCKETS_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


BUFFER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


ETHERNET_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


ICMP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


IP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\loopback.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\debug.h


LOOPBACK_DEP = c:\work\swssock\src\loopback.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


MESSAGEQ_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


RAWIP_DEP = c:\work\swssock\src\rawip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


STARTUP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\rawip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\thread.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


TIMER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


UDP_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


TCP_DEP = c:\work\swssock\src\platform.h


TCP_DEP = c:\work\swssock\src\debug.h


MAIN_DEP = c:\work\swssock\apps\httpd\httpd.h \
	c:\work\swssock\apps\httpd\httpfile.h \
	c:\work\swssock\apps\httpd\httplfg.h \
	c:\work\swssock\apps\httpd\httpv4l.h \
	c:\work\swssock\apps\httpd\httpprxy.h \
	c:\work\swssock\apps\httpd\httpcgi.h


DEQUE_DEP = c:\work\swssock\src\deque.h


DEQUE_DEP = c:\work\swssock\src\debug.h


DHCP_DEP = c:\work\swssock\src\debug.h \
	c:\work\swssock\src\platform.h


IGMP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SLIP_DEP = c:\work\swssock\src\slip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


THREAD_DEP = c:\work\swssock\src\thread.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


P2P_DEP = c:\work\swssock\src\p2p.h


P2P_DEP = c:\work\swssock\src\slip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


PPP_DEP = c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


PCCOMMS_DEP =

GEF_DEP = c:\work\swssock\src\debug.h


all:	$(PROJ).EXE $(PROJ).BSC

HTTPD.OBJ:	..\HTTPD.C $(HTTPD_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\HTTPD.C

HTTPFILE.OBJ:	..\HTTPFILE.C $(HTTPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\HTTPFILE.C

SOCKETS.OBJ:	..\..\..\SRC\SOCKETS.C $(SOCKETS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\SOCKETS.C

BUFFER.OBJ:	..\..\..\SRC\BUFFER.C $(BUFFER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\BUFFER.C

ETHERNET.OBJ:	..\..\..\SRC\ETHERNET.C $(ETHERNET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\ETHERNET.C

ICMP.OBJ:	..\..\..\SRC\ICMP.C $(ICMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\ICMP.C

IP.OBJ:	..\..\..\SRC\IP.C $(IP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\IP.C

LOOPBACK.OBJ:	..\..\..\SRC\LOOPBACK.C $(LOOPBACK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\LOOPBACK.C

MESSAGEQ.OBJ:	..\..\..\SRC\MESSAGEQ.C $(MESSAGEQ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\MESSAGEQ.C

RAWIP.OBJ:	..\..\..\SRC\RAWIP.C $(RAWIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\RAWIP.C

STARTUP.OBJ:	..\..\..\SRC\STARTUP.C $(STARTUP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\STARTUP.C

TIMER.OBJ:	..\..\..\SRC\TIMER.C $(TIMER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\TIMER.C

UDP.OBJ:	..\..\..\SRC\UDP.C $(UDP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\UDP.C

DOS16.OBJ:	..\..\..\SRC\DOS16\DOS16.C $(DOS16_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\DOS16.C

TCP.OBJ:	..\..\..\SRC\TCP.C $(TCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\TCP.C

MAIN.OBJ:	..\MAIN.C $(MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\MAIN.C

PACKET.OBJ:	..\..\..\SRC\DOS16\PACKET.C $(PACKET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\PACKET.C

DEQUE.OBJ:	..\..\..\SRC\DEQUE.C $(DEQUE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DEQUE.C

DHCP.OBJ:	..\..\..\SRC\DHCP.C $(DHCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DHCP.C

IGMP.OBJ:	..\..\..\SRC\IGMP.C $(IGMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\IGMP.C

SLIP.OBJ:	..\..\..\SRC\SLIP.C $(SLIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\SLIP.C

THREAD.OBJ:	..\..\..\SRC\THREAD.C $(THREAD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\THREAD.C

SWS_CFG.OBJ:	..\..\..\LIBS\SWS_CFG.C $(SWS_CFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_CFG.C

P2P.OBJ:	..\..\..\SRC\P2P.C $(P2P_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\P2P.C

PPP.OBJ:	..\..\..\SRC\PPP.C $(PPP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\PPP.C

PCCOMMS.OBJ:	..\..\..\SRC\DOS16\PCCOMMS.C $(PCCOMMS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\DOS16\PCCOMMS.C

GEF.OBJ:	..\..\..\SRC\GEF.C $(GEF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\SRC\GEF.C

SWS_MD5.OBJ:	..\..\..\LIBS\SWS_MD5.C $(SWS_MD5_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_MD5.C

SWS_BA64.OBJ:	..\..\..\LIBS\SWS_BA64.C $(SWS_BA64_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_BA64.C

SWS_STR.OBJ:	..\..\..\LIBS\SWS_STR.C $(SWS_STR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_STR.C

SWS_TIME.OBJ:	..\..\..\LIBS\SWS_TIME.C $(SWS_TIME_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_TIME.C


$(PROJ).EXE::	HTTPD.OBJ HTTPFILE.OBJ SOCKETS.OBJ BUFFER.OBJ ETHERNET.OBJ ICMP.OBJ \
	IP.OBJ LOOPBACK.OBJ MESSAGEQ.OBJ RAWIP.OBJ STARTUP.OBJ TIMER.OBJ UDP.OBJ DOS16.OBJ \
	TCP.OBJ MAIN.OBJ PACKET.OBJ DEQUE.OBJ DHCP.OBJ IGMP.OBJ SLIP.OBJ THREAD.OBJ SWS_CFG.OBJ \
	P2P.OBJ PPP.OBJ PCCOMMS.OBJ GEF.OBJ SWS_MD5.OBJ SWS_BA64.OBJ SWS_STR.OBJ SWS_TIME.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
HTTPD.OBJ +
HTTPFILE.OBJ +
SOCKETS.OBJ +
BUFFER.OBJ +
ETHERNET.OBJ +
ICMP.OBJ +
IP.OBJ +
LOOPBACK.OBJ +
MESSAGEQ.OBJ +
RAWIP.OBJ +
STARTUP.OBJ +
TIMER.OBJ +
UDP.OBJ +
DOS16.OBJ +
TCP.OBJ +
MAIN.OBJ +
PACKET.OBJ +
DEQUE.OBJ +
DHCP.OBJ +
IGMP.OBJ +
SLIP.OBJ +
THREAD.OBJ +
SWS_CFG.OBJ +
P2P.OBJ +
PPP.OBJ +
PCCOMMS.OBJ +
GEF.OBJ +
SWS_MD5.OBJ +
SWS_BA64.OBJ +
SWS_STR.OBJ +
SWS_TIME.OBJ +
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
