# Microsoft Developer Studio Project File - Name="sws_sock" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sws_sock - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sws_sock.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sws_sock.mak" CFG="sws_sock - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sws_sock - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sws_sock - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sws_sock - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SwsSock_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G5 /MD /W4 /O2 /I ".." /I "..\..\..\src\win32" /I "..\..\..\src" /I "..\..\..\include" /D "NDEBUG" /D SWS_USE_SOCKLIB=1 /D "SWS_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib winmm.lib /nologo /version:1.0 /dll /map /machine:I386 /RELEASE
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "sws_sock - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SwsSock_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W4 /Gm /Zi /Od /Gf /Gy /I ".." /I "..\..\..\src\win32" /I "..\..\..\src" /I "..\..\..\include" /D "_CRTDBG_MAP_ALLOC" /D "_DEBUG" /D SWS_USE_SOCKLIB=1 /D "SWS_DLL" /D OPT_DUMP=1 /D OPT_DUMP_ENET=1 /D OPT_DUMP_PPP=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /FD /TP /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /version:1.0 /dll /map /debug /machine:I386
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "sws_sock - Win32 Release"
# Name "sws_sock - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Win32 sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\ioctls.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\packet32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\packet32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\raw32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\raw32.h
# End Source File
# Begin Source File

SOURCE=.\sws_sock.def
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\w32comms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\w32comms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\win32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\ws2.c
# End Source File
# End Group
# Begin Group "Platforms"

# PROP Default_Filter ""
# Begin Group "DJGPP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\SRC\DJGPP\djgpp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DJGPP\packet.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DJGPP\pccomms.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "DOS16"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\SRC\DOS16\dos16.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DOS16\packet.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DOS16\pccomms.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Cygwin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\SRC\cygwin\cygwin.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Watcom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\watcom\DOS32\config.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\watcom\dpmi.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\watcom\packet.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\watcom\pccomms.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\watcom\swsck16.mak
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\watcom\swsck32.mak
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\watcom\swsckw32.mak
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\watcom\swsckw32dll.mak
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\watcom\wcdos32.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\..\SRC\buffer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\deque.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\dhcp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\ethernet.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\fakenic.c

!IF  "$(CFG)" == "sws_sock - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sws_sock - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\fakep2p.c

!IF  "$(CFG)" == "sws_sock - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sws_sock - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\gef.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\icmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\igmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\ip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\loopback.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\messageq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\p2p.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\ppp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\rawip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\slip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\sockets.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\startup.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_cfg.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_dmpe.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_dmpi.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_dmpp.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_dns.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_dump.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_md5.c
# End Source File
# Begin Source File

SOURCE=..\..\sws_str.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\tcp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\thread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\timer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\udp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\deque.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\fakenic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\fakep2p.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\ip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\loopback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\p2p.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\packet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\pccomms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\ppp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\rawip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\slip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_buf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dhcp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dlnk.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dmpe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dmpi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dmpp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dns.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dump.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_enet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_gef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_host.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_icmp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_igmp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_ip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_msgq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_p2p.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_ppp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_rip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_smb.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_sock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_str.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_tcp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_thrd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_timr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_tprt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_udp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\thread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\sws_sock.rc
# End Source File
# End Group
# Begin Group "RFCs"

# PROP Default_Filter ""
# Begin Group "Routing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1812.TXT
# End Source File
# End Group
# Begin Group "PPP"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\..\Rfc\ppp-numbers.txt"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1144.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1321.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1332.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1334.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1570.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1877.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1962.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC1994.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC2153.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC2433.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC2509.TXT
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\rfc3241.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\std\std51.txt
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\..\Rfc\RFC2581.TXT
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=..\..\..\c_api.txt
# End Source File
# End Group
# End Target
# End Project
