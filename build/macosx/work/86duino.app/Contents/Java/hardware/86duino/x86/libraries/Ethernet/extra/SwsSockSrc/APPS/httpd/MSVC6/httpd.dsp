# Microsoft Developer Studio Project File - Name="httpd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=httpd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "httpd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "httpd.mak" CFG="httpd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "httpd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "httpd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "httpd - Win32 Release"

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
# ADD CPP /nologo /G5 /MD /W4 /Ox /Ot /Oa /Og /Oi /Gf /Gy /I ".." /I "..\..\..\include" /I "..\lfgsdk\inc" /D "NDEBUG" /D SWS_USE_SOCKLIB=1 /D PACKET_DUMP=1 /D OPT_HTTP_CAMERA=0 /D "_TMG_WIN32" /D "_LFG_WIN32" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib winmm.lib delayimp.lib /nologo /subsystem:console /machine:I386 /delay:nobind /delayload:lfgw32.dll /delayload:tmgwin32.dll
# SUBTRACT LINK32 /profile /pdb:none /map

!ELSEIF  "$(CFG)" == "httpd - Win32 Debug"

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
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Zi /Od /Gf /Gy /I ".." /I "..\..\..\include" /I "..\lfgsdk\inc" /D "_DEBUG" /D SWS_USE_SOCKLIB=1 /D PACKET_DUMP=1 /D OPT_HTTP_CAMERA=0 /D "_TMG_WIN32" /D "_LFG_WIN32" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib kernel32.lib winmm.lib delayimp.lib /nologo /subsystem:console /debug /machine:I386 /delay:nobind /delayload:lfgw32.dll /delayload:tmgwin32.dll
# SUBTRACT LINK32 /profile /map /nodefaultlib

!ENDIF 

# Begin Target

# Name "httpd - Win32 Release"
# Name "httpd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\httpd.c
# End Source File
# Begin Source File

SOURCE=..\httpfile.c
# End Source File
# Begin Source File

SOURCE=..\httplfg.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\LFGSDK\SRC\LFG_TMG.C
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\sws_ba64.c
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\sws_time.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\httpd.h
# End Source File
# Begin Source File

SOURCE=..\httpfile.h
# End Source File
# Begin Source File

SOURCE=..\httplfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_ba64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_str.h
# End Source File
# Begin Source File

SOURCE=..\..\..\INCLUDE\sws_time.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "RFCs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=c:\Rfc\RFC1738.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\RFC1866.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\RFC1945.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\RFC2045.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\RFC2068.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\RFC2616.TXT
# End Source File
# Begin Source File

SOURCE=c:\Rfc\rfc2774.txt
# End Source File
# Begin Source File

SOURCE=c:\Rfc\rfc2822.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\sws_sock.ini
# End Source File
# Begin Source File

SOURCE=..\LFGSDK\LIB\Lfgw32.lib
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\LFGSDK\LIB\TMGWIN32.lib
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\WIN32\DLL\sws_sock.lib
# End Source File
# End Target
# End Project
