# Microsoft Visual C++ generated build script - Do not modify

PROJ = HTTPD
DEBUG = 0
PROGTYPE = 6
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
CFLAGS_D_DEXE = /nologo /Gs /G2 /Gy /W4 /Gf /Zi /AL /Od /D "OPT_HTTP_TINY"="1" /D "SWS_USE_SOCKLIB"="1" /D "_DEBUG" /D "_DOS16" /I "." /I ".." /I "..\..\..\include" /FR /Fd"HTTPD.PDB"
CFLAGS_R_DEXE = /nologo /f- /Gs /G2 /Gy /W4 /Gf /AL /O1 /D "OPT_HTTP_TINY"="1" /D "SWS_USE_SOCKLIB"="1" /D "NDEBUG" /D "_DOS16" /I "." /I ".." /I "..\..\..\include" 
LFLAGS_D_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /EXEPACK /ONERROR:NOEXE 
LFLAGS_R_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /EXEPACK /ONERROR:NOEXE /MAP 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = SWSSOCK.LIB 
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
SBRS = HTTPD.SBR \
		HTTPFILE.SBR \
		MAIN.SBR \
		SWS_STR.SBR


SWSSOCK_DEP = 

HTTPD_DEP = c:\work\swssock\apps\httpd\httpd.h


HTTPFILE_DEP = c:\work\swssock\apps\httpd\httpfile.h \
	c:\work\swssock\apps\httpd\httpd.h


MAIN_DEP = c:\work\swssock\apps\httpd\httpd.h \
	c:\work\swssock\apps\httpd\httpfile.h \
	c:\work\swssock\apps\httpd\httplfg.h \
	c:\work\swssock\apps\httpd\httpv4l.h \
	c:\work\swssock\apps\httpd\httpprxy.h \
	c:\work\swssock\apps\httpd\httpcgi.h


all:	$(PROJ).EXE

HTTPD.OBJ:	..\HTTPD.C $(HTTPD_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\HTTPD.C

HTTPFILE.OBJ:	..\HTTPFILE.C $(HTTPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\HTTPFILE.C

MAIN.OBJ:	..\MAIN.C $(MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\MAIN.C

SWS_STR.OBJ:	..\..\..\LIBS\SWS_STR.C $(SWS_STR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_STR.C

$(PROJ).EXE::	HTTPD.OBJ HTTPFILE.OBJ MAIN.OBJ SWS_STR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
HTTPD.OBJ +
HTTPFILE.OBJ +
MAIN.OBJ +
SWS_STR.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
C:\MSVC\LIB\+
C:\MSVC\MFC\LIB\+
SWSSOCK.LIB+
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
