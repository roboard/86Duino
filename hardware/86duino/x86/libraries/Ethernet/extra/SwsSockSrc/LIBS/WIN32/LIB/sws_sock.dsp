# Microsoft Developer Studio Project File - Name="sws_sock" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

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
!MESSAGE "sws_sock - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sws_sock - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
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
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W4 /O1 /I ".." /I "..\..\..\src\win32" /I "..\..\..\src" /I "..\..\..\include" /D "NDEBUG" /D SWS_USE_SOCKLIB=1 /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

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
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Zi /Od /Gf /Gy /I ".." /I "..\..\..\src\win32" /I "..\..\..\src" /I "..\..\..\include" /D "_DEBUG" /D "_CRTDBG_MAP_ALLOC" /D SWS_USE_SOCKLIB=1 /D OPT_DUMP=1 /D OPT_DUMP_PPP=1 /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sws_sock - Win32 Release"
# Name "sws_sock - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Platforms"

# PROP Default_Filter ""
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

SOURCE=..\..\..\SRC\DOS16\packet.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DOS16\pccomms.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DOS16\pccomms.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
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

SOURCE=..\..\..\SRC\DJGPP\packet.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DJGPP\pccomms.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\DJGPP\pccomms.h
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
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\DRVRS\Win32\ioctls.h
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

SOURCE=..\..\..\SRC\WIN32\w32comms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\w32comms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\SRC\WIN32\win32.c
# End Source File
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

SOURCE=..\..\..\SRC\WIN32\ioctls.h
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

SOURCE=..\..\..\INCLUDE\sws_rpc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_slip.h
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
# End Group
# End Target
# End Project
