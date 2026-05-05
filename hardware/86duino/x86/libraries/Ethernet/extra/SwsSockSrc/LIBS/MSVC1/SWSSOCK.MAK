# Microsoft Visual C++ generated build script - Do not modify

PROJ = SWSSOCK
DEBUG = 0
PROGTYPE = 4
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\SWSSOCK\LIBS\MSVC1\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = BUFFER.C    
FIRSTCPP =             
RC = rc
CFLAGS_D_LIB = /nologo /Gs /G2 /Gy /W4 /Gf /Z7 /ALw /Gt8 /O1 /Ob1 /D "SWS_USE_SOCKLIB"="1" /D "_DEBUG" /D "NDEBUG" /D "_DOS16" /I "." /I "..\..\src\dos16" /I "..\..\src" /I "..\..\include" /FR 
CFLAGS_R_LIB = /nologo /f- /Gs /G2 /Gy /W4 /Gf /ALw /O1 /Ob1 /D "SWS_USE_SOCKLIB"="1" /D "NDEBUG" /D "_DOS16" /I "." /I "..\..\src\dos16" /I "..\..\src" /I "..\..\include" /Fc 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_LIB)
LFLAGS = 
LIBS = 
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_LIB)
LFLAGS = 
LIBS = 
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = BUFFER.SBR \
		DEQUE.SBR \
		DHCP.SBR \
		ETHERNET.SBR \
		ICMP.SBR \
		IGMP.SBR \
		IP.SBR \
		LOOPBACK.SBR \
		MESSAGEQ.SBR \
		RAWIP.SBR \
		SLIP.SBR \
		SOCKETS.SBR \
		STARTUP.SBR \
		TCP.SBR \
		THREAD.SBR \
		TIMER.SBR \
		UDP.SBR \
		DOS16.SBR \
		PACKET.SBR \
		PCCOMMS.SBR \
		SWS_CFG.SBR \
		SWS_DNS.SBR \
		P2P.SBR \
		PPP.SBR \
		SWS_MD5.SBR \
		FAKENIC.SBR \
		FAKEP2P.SBR \
		GEF.SBR \
		SWS_DMPE.SBR \
		SWS_DMPI.SBR \
		SWS_DMPP.SBR \
		SWS_DUMP.SBR


BUFFER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


DEQUE_DEP = c:\work\swssock\src\deque.h \
	c:\work\swssock\src\debug.h


DHCP_DEP = c:\work\swssock\src\debug.h \
	c:\work\swssock\src\platform.h


ETHERNET_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


ICMP_DEP = c:\work\swssock\src\ip.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


IGMP_DEP = c:\work\swssock\src\ip.h \
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


SLIP_DEP = c:\work\swssock\src\slip.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SOCKETS_DEP = c:\work\swssock\src\ip.h \
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


TIMER_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


UDP_DEP = c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SWS_CFG_DEP = c:\work\swssock\include\sws_cfg.h


SWS_CFG_DEP = c:\work\swssock\include\sws_host.h


SWS_DNS_DEP = c:\work\swssock\include\sws_dns.h


SWS_DNS_DEP = c:\work\swssock\include\sws_host.h


SWS_DNS_DEP = c:\work\swssock\include\sws_net.h \
	c:\work\swssock\include\sws_sock.h \
	c:\work\swssock\include\sws_host.h \
	c:\work\swssock\include\sws_gef.h \
	c:\msvc\include\winsock.h \
	c:\work\swssock\include\sws_cfg.h


P2P_DEP = c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\slip.h \
	c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\deque.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


PPP_DEP = c:\work\swssock\src\ppp.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\include\sws_md5.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


SWS_MD5_DEP = c:\work\swssock\include\sws_md5.h


FAKENIC_DEP = c:\work\swssock\src\fakenic.h \
	c:\work\swssock\src\debug.h


FAKEP2P_DEP = c:\work\swssock\src\fakep2p.h \
	c:\work\swssock\src\p2p.h \
	c:\work\swssock\src\platform.h \
	c:\work\swssock\src\debug.h


GEF_DEP = c:\work\swssock\src\debug.h


SWS_DMPE_DEP = c:\work\swssock\include\sws_dmpe.h \
	c:\work\swssock\include\sws_host.h \
	c:\work\swssock\include\sws_gef.h \
	c:\work\swssock\include\sws_buf.h \
	c:\work\swssock\include\sws_cfg.h \
	c:\work\swssock\include\sws_sock.h \
	c:\work\swssock\include\sws_enet.h \
	c:\work\swssock\include\sws_ppp.h \
	c:\work\swssock\include\sws_dmpp.h


SWS_DMPI_DEP = c:\work\swssock\include\sws_dmpi.h \
	c:\work\swssock\include\sws_host.h \
	c:\work\swssock\include\sws_buf.h \
	c:\work\swssock\include\sws_gef.h \
	c:\work\swssock\include\sws_cfg.h \
	c:\work\swssock\include\sws_sock.h \
	c:\work\swssock\include\sws_ip.h \
	c:\work\swssock\include\sws_icmp.h \
	c:\work\swssock\include\sws_igmp.h \
	c:\work\swssock\include\sws_udp.h \
	c:\work\swssock\include\sws_tcp.h \
	c:\work\swssock\include\sws_dhcp.h \
	c:\work\swssock\include\sws_dns.h \
	c:\work\swssock\include\sws_smb.h \
	c:\work\swssock\include\sws_rip.h


SWS_DMPP_DEP = c:\work\swssock\include\sws_dmpp.h \
	c:\work\swssock\include\sws_host.h \
	c:\work\swssock\include\sws_gef.h \
	c:\work\swssock\include\sws_buf.h \
	c:\work\swssock\include\sws_cfg.h \
	c:\work\swssock\include\sws_sock.h \
	c:\work\swssock\include\sws_ppp.h


SWS_DUMP_DEP = c:\work\swssock\include\sws_dump.h \
	c:\work\swssock\include\sws_gef.h \
	c:\work\swssock\include\sws_host.h \
	c:\work\swssock\include\sws_cfg.h \
	c:\work\swssock\include\sws_dmpi.h \
	c:\work\swssock\include\sws_buf.h \
	c:\work\swssock\include\sws_dmpe.h \
	c:\work\swssock\include\sws_dmpp.h \
	c:\work\swssock\include\sws_sock.h


all:	$(PROJ).LIB

BUFFER.OBJ:	..\..\SRC\BUFFER.C $(BUFFER_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\..\SRC\BUFFER.C

DEQUE.OBJ:	..\..\SRC\DEQUE.C $(DEQUE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\DEQUE.C

DHCP.OBJ:	..\..\SRC\DHCP.C $(DHCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\DHCP.C

ETHERNET.OBJ:	..\..\SRC\ETHERNET.C $(ETHERNET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\ETHERNET.C

ICMP.OBJ:	..\..\SRC\ICMP.C $(ICMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\ICMP.C

IGMP.OBJ:	..\..\SRC\IGMP.C $(IGMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\IGMP.C

IP.OBJ:	..\..\SRC\IP.C $(IP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\IP.C

LOOPBACK.OBJ:	..\..\SRC\LOOPBACK.C $(LOOPBACK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\LOOPBACK.C

MESSAGEQ.OBJ:	..\..\SRC\MESSAGEQ.C $(MESSAGEQ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\MESSAGEQ.C

RAWIP.OBJ:	..\..\SRC\RAWIP.C $(RAWIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\RAWIP.C

SLIP.OBJ:	..\..\SRC\SLIP.C $(SLIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\SLIP.C

SOCKETS.OBJ:	..\..\SRC\SOCKETS.C $(SOCKETS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\SOCKETS.C

STARTUP.OBJ:	..\..\SRC\STARTUP.C $(STARTUP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\STARTUP.C

TCP.OBJ:	..\..\SRC\TCP.C $(TCP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\TCP.C

THREAD.OBJ:	..\..\SRC\THREAD.C $(THREAD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\THREAD.C

TIMER.OBJ:	..\..\SRC\TIMER.C $(TIMER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\TIMER.C

UDP.OBJ:	..\..\SRC\UDP.C $(UDP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\UDP.C

DOS16.OBJ:	..\..\SRC\DOS16\DOS16.C $(DOS16_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\DOS16\DOS16.C

PACKET.OBJ:	..\..\SRC\DOS16\PACKET.C $(PACKET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\DOS16\PACKET.C

PCCOMMS.OBJ:	..\..\SRC\DOS16\PCCOMMS.C $(PCCOMMS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\DOS16\PCCOMMS.C

SWS_CFG.OBJ:	..\..\LIBS\SWS_CFG.C $(SWS_CFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_CFG.C

SWS_DNS.OBJ:	..\..\LIBS\SWS_DNS.C $(SWS_DNS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_DNS.C

P2P.OBJ:	..\..\SRC\P2P.C $(P2P_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\P2P.C

PPP.OBJ:	..\..\SRC\PPP.C $(PPP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\PPP.C

SWS_MD5.OBJ:	..\..\LIBS\SWS_MD5.C $(SWS_MD5_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_MD5.C

FAKENIC.OBJ:	..\..\SRC\FAKENIC.C $(FAKENIC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\FAKENIC.C

FAKEP2P.OBJ:	..\..\SRC\FAKEP2P.C $(FAKEP2P_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\FAKEP2P.C

GEF.OBJ:	..\..\SRC\GEF.C $(GEF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SRC\GEF.C

SWS_DMPE.OBJ:	..\..\LIBS\SWS_DMPE.C $(SWS_DMPE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_DMPE.C

SWS_DMPI.OBJ:	..\..\LIBS\SWS_DMPI.C $(SWS_DMPI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_DMPI.C

SWS_DMPP.OBJ:	..\..\LIBS\SWS_DMPP.C $(SWS_DMPP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_DMPP.C

SWS_DUMP.OBJ:	..\..\LIBS\SWS_DUMP.C $(SWS_DUMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\LIBS\SWS_DUMP.C

$(PROJ).LIB::	BUFFER.OBJ DEQUE.OBJ DHCP.OBJ ETHERNET.OBJ ICMP.OBJ IGMP.OBJ IP.OBJ \
	LOOPBACK.OBJ MESSAGEQ.OBJ RAWIP.OBJ SLIP.OBJ SOCKETS.OBJ STARTUP.OBJ TCP.OBJ THREAD.OBJ \
	TIMER.OBJ UDP.OBJ DOS16.OBJ PACKET.OBJ PCCOMMS.OBJ SWS_CFG.OBJ SWS_DNS.OBJ P2P.OBJ \
	PPP.OBJ SWS_MD5.OBJ FAKENIC.OBJ FAKEP2P.OBJ GEF.OBJ SWS_DMPE.OBJ SWS_DMPI.OBJ SWS_DMPP.OBJ \
	SWS_DUMP.OBJ $(OBJS_EXT)
	echo >NUL @<<$(PROJ).CRF
$@ /PAGESIZE:64
y
+BUFFER.OBJ &
+DEQUE.OBJ &
+DHCP.OBJ &
+ETHERNET.OBJ &
+ICMP.OBJ &
+IGMP.OBJ &
+IP.OBJ &
+LOOPBACK.OBJ &
+MESSAGEQ.OBJ &
+RAWIP.OBJ &
+SLIP.OBJ &
+SOCKETS.OBJ &
+STARTUP.OBJ &
+TCP.OBJ &
+THREAD.OBJ &
+TIMER.OBJ &
+UDP.OBJ &
+DOS16.OBJ &
+PACKET.OBJ &
+PCCOMMS.OBJ &
+SWS_CFG.OBJ &
+SWS_DNS.OBJ &
+P2P.OBJ &
+PPP.OBJ &
+SWS_MD5.OBJ &
+FAKENIC.OBJ &
+FAKEP2P.OBJ &
+GEF.OBJ &
+SWS_DMPE.OBJ &
+SWS_DMPI.OBJ &
+SWS_DMPP.OBJ &
+SWS_DUMP.OBJ &
;
<<
	if exist $@ del $@
	lib @$(PROJ).CRF

$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
