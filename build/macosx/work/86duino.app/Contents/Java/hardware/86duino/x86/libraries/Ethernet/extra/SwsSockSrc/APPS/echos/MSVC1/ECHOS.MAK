# Microsoft Visual C++ generated build script - Do not modify

PROJ = ECHOS
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\SWSSOCK\APPS\ECHOS\MSVC1\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = ECHO.C      
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /Gs /G2 /Gy /W4 /Gf /Zi /AL /Od /D "SWS_USE_SOCKLIB"="1" /D "PACKET_DUMP"="1" /D "_DEBUG" /D "_DOS16" /I "." /I ".." /I "..\..\..\include" /FR /Fd"ECHOS.PDB"
CFLAGS_R_DEXE = /nologo /f- /Gs /G2 /Gy /W4 /Gf /AL /O1 /D "SWS_USE_SOCKLIB"="1" /D "PACKET_DUMP"="1" /D "NDEBUG" /D "_DOS16" /I "." /I ".." /I "..\..\..\include" 
LFLAGS_D_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /FARCALL /STACK:5120 /ONERROR:NOEXE 
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
SBRS = ECHO.SBR \
		ECHOD.SBR \
		SWS_STR.SBR


SWSSOCK_DEP = 

ECHO_DEP = c:\work\swssock\apps\echos\echod.h


ECHOD_DEP = c:\work\swssock\apps\echos\echod.h


all:	$(PROJ).EXE

ECHO.OBJ:	..\ECHO.C $(ECHO_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\ECHO.C

ECHOD.OBJ:	..\ECHOD.C $(ECHOD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\ECHOD.C

SWS_STR.OBJ:	..\..\..\LIBS\SWS_STR.C $(SWS_STR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\..\LIBS\SWS_STR.C

$(PROJ).EXE::	ECHO.OBJ ECHOD.OBJ SWS_STR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
ECHO.OBJ +
ECHOD.OBJ +
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
