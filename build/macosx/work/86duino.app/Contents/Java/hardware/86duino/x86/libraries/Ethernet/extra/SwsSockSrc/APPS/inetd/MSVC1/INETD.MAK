# Microsoft Visual C++ generated build script - Do not modify

PROJ = INETD
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\SWSSOCK\APPS\INETD\MSVC1\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = INETD.C     
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /Gs /G2 /W4 /Gf /Zi /ALw /Od /D "SWS_USE_SOCKLIB"="1" /D "INETD_OPT"="1" /D "OPT_HTTP_TINY"="1" /D "_DEBUG" /D "_DOS16" /I "." /I "..\..\httpd" /I "..\..\echos" /I "..\..\..\include" /Fd"INETD.PDB"
CFLAGS_R_DEXE = /nologo /f- /Gs /G2 /W4 /Gf /ALw /O1 /Ob1 /D "SWS_USE_SOCKLIB"="1" /D "INETD_OPT"="1" /D "OPT_HTTP_TINY"="1" /D "NDEBUG" /D "_DOS16" /I "." /I "..\..\httpd" /I "..\..\echos" /I "..\..\..\include" 
LFLAGS_D_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /EXEPACK /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /EXEPACK /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = ..\..\..\LIBS\MSVC1\SWSSOCK.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = INETD.SBR \
		ECHOD.SBR \
		HTTPD.SBR \
		HTTPFILE.SBR \
		SWS_STR.SBR


SWSSOCK_DEP = 

ECHOD_DEP = c:\work\swssock\apps\echos\echod.h


HTTPD_DEP = c:\work\swssock\apps\httpd\httpd.h


HTTPFILE_DEP = c:\work\swssock\apps\httpd\httpfile.h


HTTPFILE_DEP = c:\work\swssock\apps\httpd\httpd.h


all:	$(PROJ).EXE

INETD.OBJ:	..\INETD.C $(INETD_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\INETD.C

ECHOD.OBJ:	..\..\ECHOS\ECHOD.C $(ECHOD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\ECHOS\ECHOD.C

HTTPD.OBJ:	..\..\HTTPD\HTTPD.C $(HTTPD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\HTTPD\HTTPD.C

HTTPFILE.OBJ:	..\..\HTTPD\HTTPFILE.C $(HTTPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\HTTPD\HTTPFILE.C

SWS_STR.OBJ:	..\..\..\LIBS\SWS_STR.C $(SWS_STR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_STR.C

$(PROJ).EXE::	INETD.OBJ ECHOD.OBJ HTTPD.OBJ HTTPFILE.OBJ SWS_STR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
INETD.OBJ +
ECHOD.OBJ +
HTTPD.OBJ +
HTTPFILE.OBJ +
SWS_STR.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
C:\MSVC\LIB\+
C:\MSVC\MFC\LIB\+
..\..\..\LIBS\MSVC1\SWSSOCK.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
