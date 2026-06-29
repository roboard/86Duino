# Microsoft Developer Studio Project File - Name="inetsvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=inetsvc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "inetsvc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "inetsvc.mak" CFG="inetsvc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "inetsvc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "inetsvc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "inetsvc - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W4 /O1 /I "..\..\ftpd" /I "..\..\httpd" /I "..\..\popd" /I "..\..\smtpd" /I "..\..\telnetd" /I "..\..\sendmail" /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D TELNETD_EXT=2 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib advapi32.lib ws2_32.lib winmm.lib /nologo /version:1.0 /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "inetsvc - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Zi /Od /Gf /Gy /I "..\..\ftpd" /I "..\..\httpd" /I "..\..\popd" /I "..\..\smtpd" /I "..\..\telnetd" /I "..\..\sendmail" /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D TELNETD_EXT=2 /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib kernel32.lib advapi32.lib ws2_32.lib winmm.lib /nologo /version:1.0 /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "inetsvc - Win32 Release"
# Name "inetsvc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\ftpd\ftpd.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# Begin Source File

SOURCE=..\..\httpd\httpd.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# Begin Source File

SOURCE=..\..\httpd\httpfile.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# Begin Source File

SOURCE=.\inetsvc.c
# End Source File
# Begin Source File

SOURCE=.\mainsvc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\popd\popd.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# Begin Source File

SOURCE=..\..\popd\popmsg.c
# End Source File
# Begin Source File

SOURCE=..\..\sendmail\sendmail.c
# End Source File
# Begin Source File

SOURCE=..\..\smtpd\smtpd.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\sws_cfg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\sws_dns.c
# End Source File
# Begin Source File

SOURCE=..\..\telnetd\telnetd.c
# ADD CPP /D "WIN32_LEAN_AND_MEAN"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\ftpd\ftpd.h
# End Source File
# Begin Source File

SOURCE=..\..\httpd\httpd.h
# End Source File
# Begin Source File

SOURCE=..\..\httpd\httpfile.h
# End Source File
# Begin Source File

SOURCE=..\..\popd\popd.h
# End Source File
# Begin Source File

SOURCE=..\..\popd\popmsg.h
# End Source File
# Begin Source File

SOURCE=..\..\sendmail\sendmail.h
# End Source File
# Begin Source File

SOURCE=..\..\smtpd\smtpd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_dns.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_host.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_thrd.h
# End Source File
# Begin Source File

SOURCE=..\..\telnetd\telnetd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\inetsvc.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\inetsvc.inf
# End Source File
# Begin Source File

SOURCE=.\INETSVC.INI
# End Source File
# End Target
# End Project
