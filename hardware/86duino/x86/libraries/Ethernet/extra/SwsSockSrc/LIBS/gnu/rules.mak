#!/usr/bin/make
# Generic GNU makefile rules
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  31 May 09  lvr  Mods for swssock
#  01  12 Mar 09  lvr  Creation
# -------------------------------------------------------------------------
#
# Instructions
# ------------
# To build debug version:
# make DEBUG=1
#
# To cross compile Mingw32 (gcc 4.2):
# make XMINGW32=1
#
# To cross compile DJGPP 2.04 (gcc 4.3):
# make XDJGPP=1
#
#$(info .VARIABLES: $(.VARIABLES))
#$(info .FEATURES: $(.FEATURES))

# Configurable options:
#
#TARGET : program to build
#SOURCES : list of object files without suffixes. may be empty
#VERBOSITY = 1 (0|1|2|3) : message verbosity
#NOTANSISOURCES : list of source files without suffixes built without -ansi -pedantic
#BSDSOURCES : list of source files without suffixes built with _BSD_SOURCE=1
#POSIXSOURCES : list of source files without suffixes built with _POSIX_C_SOURCE=199506L
#SRCPATH : space separated list of paths to search for source files
#INCPATH : space separated list of paths to search for source files
#LIBPATH : space separated list of paths to search for library files
#PREFIX : Prefix prepended to all build tools
#CPPFLAGS : cpp flags
#CFLAGS : cc flags
#STRICT = -ansi -pedantic
#CXXFLAGS : c++ flags
#CXXEXCEPTIONS = (|1)
#CXXRTTI = (|1)
#OPTIMISE = -O2|-ggdb3 -g3
#FASTMATH = (|1)
#PROFILE = (|1|gen|use)
#NDEBUG = 1 (|0|1) : 1 to disable assert(...)
#LDFLAGS : linker flags
#LIBS : list of user libraries for linker
#STATIC = (|1) 1= link static libraries
#EXPORTSYMS = (|1) 1= add all symbols to the dynamic symbol table
#MCHECK = (|1) to include malloc tracing library
#AUTODEPENDS = 0 : (0|1)
#SRCS : list of source files for depend
#ARCHIVE : space separated list of files to add to zip/tar archive
#DJSTUB : Protected mode stub for stanalone DPMI apps
#
# Goals:
#
#all
#$(TARGET)
#syms : build $(TARGET) and split off symbols
#standalone : DJGPP $(TARGET) linked with DPMI server
#tar, zip : source archives
#clean
#depends : Rebuild auto dependencies
#depend : append dependencies to makefile

ifndef TARGET
 TARGET := $(firstword $(SOURCES))
endif
ifndef SOURCES
 SOURCES := $(basename $(TARGET))
endif

# The platform
#
ECHO = echo
TRUE = true
STRIP = strip
OBJCOPY = objcopy
CPP = cpp
touch = touch $(1)
rm = $(RM) $(1)
rd = $(RM) -r $(1)
md = mkdir -p $(1)
exe = $(1)
NULL = /dev/null
ifdef DJGPP
 # DOS/Win32 host
 exe = $(1).exe
 ifeq (,$(suffix $(TARGET)))
  TARGET := $(call exe,$(TARGET))
 endif
 NULL = nul
 ECHO = djecho
 TRUE = rem
 touch = echo. > $(subst /,\,$(1))
 rm = if exist $(subst /,\,$(1)) del $(subst /,\,$(1))
 md = mkdir $(subst /,\,$(1))
 ifdef SYSTEMROOT
  # No deltree on Windows NT, 2K, XP ...
  rd = rmdir $(subst /,\,$(1)) > $(NULL)
 else
  rd = deltree /y $(subst /,\,$(1)) > $(NULL)
 endif
else
ifdef XDJGPP
 # Cross compile -> djgpp
 PREFIX ?= i586-pc-msdosdjgpp-
 CC = gcc
 exe = $(1).exe
 ifeq (,$(suffix $(TARGET)))
  TARGET := $(call exe,$(TARGET))
 endif
else
ifdef XMINGW32
 # Cross compile -> win32
 PREFIX ?= i586-mingw32msvc-
 exe = $(1).exe
 ifeq (,$(suffix $(TARGET)))
  TARGET := $(call exe,$(TARGET))
 endif
 .LIBPATTERNS = lib%.a %.lib
 DLLTOOL = $(PREFIX)dlltool
else
ifdef XBCC
 # Cross compile -> DOS
 CC = bcc -ansi -3 -Md -L/usr/lib/bcc
 CXX := $(CC)
 STRICT =
 exe = $(1).exe
 ifeq (,$(suffix $(TARGET)))
  TARGET := $(call exe,$(TARGET))
 endif
 AUTODEPENDS = 0
 OPTIMISE =
endif
endif
endif
endif

########################
# Paths

ifdef DEBUG
 OBJ = ./$(PREFIX)dbg
else
 OBJ = ./$(PREFIX)rel
endif
vpath %.o $(OBJ)
vpath %.d $(OBJ)
src2o = $(addsuffix .o,$(addprefix $(OBJ)/,$(notdir $(1))))
src2d = $(addsuffix .d,$(addprefix $(OBJ)/,$(notdir $(1))))

#SRCPATH ?= .
vpath %.c $(SRCPATH)
vpath %.cpp $(SRCPATH)
vpath %.cc $(SRCPATH)
vpath %.S $(SRCPATH)
vpath %.s $(SRCPATH)
vpath %.s $(OBJ)

#INCPATH ?= .
vpath %.h $(INCPATH)
INCLUDE = $(addprefix -I,$(INCPATH))

#LIBPATH ?= .
vpath %.a $(LIBPATH)
vpath %.so $(LIBPATH)
LDPATH = $(addprefix -L,$(LIBPATH))
ifdef XDJGPP
 vpath %.a /usr/i586-pc-msdosdjgpp/lib
else
ifdef XMINGW32
 vpath %.lib $(LIBPATH)
 vpath %.a /usr/i586-mingw32msvc/lib
endif
endif


########################
# Tools

# C compiler
CC := $(PREFIX)$(CC)

# C/C++ Warnings
STRICT ?= -ansi -pedantic
WARN += -Wall -W -Wunused
WARN += -Wpointer-arith -Wwrite-strings -Wcast-qual -Wcast-align
WARN += -Wshadow -Wfloat-equal -Wsign-compare
WARN += -Wmultichar -Wunknown-pragmas
WARN += -Wformat=2
#WARN += -Wconversion

# C only warnings
CWARN += -Wbad-function-cast
#CWARN += -Wnested-externs # NB Causes warning for STATIC_ASSERT
CWARN += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations

ifndef XBCC
CFLAGS += $(WARN) $(CWARN)
endif

# C++ compiler
CXX := $(PREFIX)$(CXX)

# C++ only warnings
CXXWARN += -ffor-scope
CXXWARN += -Woverloaded-virtual
CXXWARN += -Wno-unreachable-code
#CXXWARN += -Weffc++
CXXWARN += -Wreorder
CXXWARN += -Wctor-dtor-privacy
#CXXWARN += -Wold-style-cast
CXXWARN += -Wstrict-null-sentinel
CXXWARN += -Wsign-promo

ifndef XBCC
CXXFLAGS += $(WARN) $(CXXWARN)
ifneq (1,$(CXXEXCEPTIONS))
 CXXFLAGS += -fno-exceptions
endif
ifneq (1,$(CXXRTTI))
 CXXFLAGS += -fno-rtti
endif
endif

# Assembler
CPP := $(PREFIX)$(CPP)
AS := $(PREFIX)$(AS)
#ASFLAGS =

# Linker
LD = $(CC)
STRIP := $(PREFIX)$(STRIP)
OBJCOPY := $(PREFIX)$(OBJCOPY)
ifeq (1,$(STATIC))
 # no shared libs
 LDFLAGS += -static
endif
ifeq (1,$(EXPORTSYMS))
 # add all symbols, not only used ones, to the dynamic symbol table
 LDFLAGS += -rdynamic
endif

# Lib
AR := $(PREFIX)$(AR)
ARFLAGS ?= qc


# Default to ndebug build
ifdef DEBUG
 # Debug options
 override CPPFLAGS += -DDEBUG
 override OPTIMISE += -ggdb3 -g3
 CFLAGS += $(OPTIMISE)
 CXXFLAGS += $(OPTIMISE)
 LDFLAGS += $(OPTIMISE)
 ASFLAGS += -g

 #Force C++ compile/link
 ifeq (cxx,$(DEBUG))
  CC = $(CXX)
  CFLAGS = $(CXXFLAGS)
  LD = $(CXX)
 endif
else
 # Release options
 NDEBUG ?= 1
 OPTIMISE ?= -O2
 CFLAGS += $(OPTIMISE)
 CXXFLAGS += $(OPTIMISE)
 LDFLAGS += $(OPTIMISE)
 ifeq (,$(findstring -g,$(OPTIMISE)))
  ifneq (1,$(PROFILE))
   ifeq (,$(findstring syms,$(MAKECMDGOALS)))
    # strip symbols
    LDFLAGS += -s
   endif
  endif
 endif
endif
ifeq (1,$(NDEBUG))
 override CPPFLAGS += -DNDEBUG
else
 override CPPFLAGS += -D_FORTIFY_SOURCE=2
endif

# Heap checking
# NB Gnu libc also supports setting 'MALLOC_CHECK_' 0= disable, 1= stderr, 2= abort
MCHECK ?= 0
ifneq (0,$(MCHECK))
 LIBS += -lmcheck
endif

# Branch profiling and optimisation
ifeq (gen,$(PROFILE))
 CFLAGS += -fprofile-generate
 CXXFLAGS += -fprofile-generate
 LDFLAGS += -fprofile-generate
else
ifeq (use,$(PROFILE))
 CFLAGS += -fprofile-use
 CXXFLAGS += -fprofile-use
 LDFLAGS += -fprofile-use
else
# gprof
ifeq (1,$(PROFILE))
 CFLAGS += -pg -g
 CXXFLAGS += -pg -g
 LDFLAGS += -pg
else
ifneq (,$(PROFILE))
 $(error Unknown PROFILE=$(PROFILE). Expected 1, gen or use)
endif
endif
endif
endif

# Fast fp - not IEEE
ifeq (1,$(FASTMATH))
 CFLAGS += -ffast-math
 CXXFLAGS += -ffast-math
endif

# Target specific variables
ifneq (,$(findstring target-specific,$(.FEATURES)))
 target-var = $(eval $(1) : $(2))
else
ifdef DJGPP
 # DJGPP make 3.79.1 has limited support for target-specific variables
 # but doesn't implement eval
 target-var = $(1) : $(2) # $(info NOTE $$(eval $(1) : $(2)))
else
 target-var = $(eval $(2)) $(info NOTE workaround for $(1) : $(2))
endif
endif
nl := '\n'
#obj-var = $(call target-var,$(call src2o,$(1)),$(2))
obj-var = $(call target-var,$(call src2o,$(1)) $(call src2d,$(1)),$(2))

# Sources with POSIX or BSD dependencies - not ANSI strict
ifneq (,$(NOTANSISOURCES))
 ifeq (1,$(words $(SOURCES)))
  STRICT =
 else
  $(call obj-var,$(NOTANSISOURCES),STRICT =)
 endif
endif
ifneq (,$(BSDSOURCES))
 ifeq (1,$(words $(SOURCES)))
  override CPPFLAGS += -D_BSD_SOURCE=1
 else
  $(call obj-var,$(BSDSOURCES),CPPFLAGS += -D_BSD_SOURCE=1)
 endif
endif
ifneq (,$(POSIXSOURCES))
 ifeq (1,$(words $(SOURCES)))
  override CPPFLAGS += -D_POSIX_C_SOURCE=199506L
 else
  $(call obj-var,$(POSIXSOURCES),CPPFLAGS += -D_POSIX_C_SOURCE=199506L)
 endif
endif

# Messages
VERBOSITY ?= 1
ifeq (0,$(VERBOSITY))
 ECHO := $(TRUE)
else
ifeq (1,$(VERBOSITY))
 CINFO =
 CXXINFO =
 LDINFO =
else
ifeq (2,$(VERBOSITY))
 CINFO = $(CPPFLAGS)
 CXXINFO = $(CPPFLAGS)
 LDINFO =
else
ifeq (3,$(VERBOSITY))
 CINFO = $(STRICT) $(CFLAGS) $(CPPFLAGS) $(INCLUDE)
 CXXINFO = $(STRICT) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE)
 LDINFO = $(LDFLAGS) $(LDPATH)
else
 $(error Invalid VERBOSITY=$(VERBOSITY). Expected 0..3)
endif
endif
endif
endif

########################
# Implicit rules

.SUFFIXES :
.SUFFIXES : .c .cpp .cc .h .S .s .o .d .a .so .dbg

$(call src2o,%) : %.c
	@$(ECHO) $(CC) $(CINFO) $(notdir $<)
	@$(CC) -c $(STRICT) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

$(call src2o,%) : %.cpp
	@$(ECHO) $(CXX) $(CXXINFO) $(notdir $<)
	@$(CXX) -c $(STRICT) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

$(call src2o,%) : %.cc
	@$(ECHO) $(CXX) $(CXXINFO) $(notdir $<)
	@$(CXX) -c $(STRICT) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

ifeq ($(TARGET),$(call exe,$(SOURCES)))
$(call exe,%) : %.c
	@$(ECHO) $(CC) $(OUTPUT_OPTION) $(CINFO) $(notdir $<) $(LDINFO) $(LIBS)
	@$(CC) $(STRICT) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $< $(LDFLAGS) $(LDPATH) $(LIBS)

$(call exe,%) : %.cpp
	@$(ECHO) $(CXX) $(OUTPUT_OPTION) $(CXXINFO) $(notdir $<) $(LDINFO) $(LIBS)
	@$(CXX) $(STRICT) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $< $(LDFLAGS) $(LDPATH) $(LIBS)

$(call exe,%) : %.cc
	@$(ECHO) $(CXX) $(OUTPUT_OPTION) $(CXXINFO) $(notdir $<) $(LDINFO) $(LIBS)
	@$(CXX) $(STRICT) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $< $(LDFLAGS) $(LDPATH) $(LIBS)
endif

# Assembler sources
$(OBJ)/%.s : %.S
	@$(ECHO) $(CPP) $(CPPFLAGS) $(notdir $<)
	@$(CPP) -E $(CPPFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

$(call src2o,%) : %.s
	@$(ECHO) $(AS) $(ASFLAGS) $(notdir $<)
	@$(AS) $(ASFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

# Analyse dependencies
$(OBJ)/%.d : %.c
	@$(ECHO) Dependencies of $(notdir $<) $(CINFO) ...
	@$(CC) $(STRICT) -MM -MG -MT $@ -MT $(@:.d=.o) $(CPPFLAGS) $(INCLUDE) $< > $@

$(OBJ)/%.d : %.cpp
	@$(ECHO) Dependencies of $(notdir $<) $(CXXINFO) ...
	@$(CXX) $(STRICT) -MM -MG -MT $@ -MT $(@:.d=.o) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $< > $@

$(OBJ)/%.d : %.cc
	@$(ECHO) Dependencies of $(notdir $<) $(CXXINFO) ...
	@$(CXX) $(STRICT) -MM -MG -MT $@ -MT $(@:.d=.o) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $< > $@

$(OBJ)/%.d : %.S
	@$(ECHO) Dependencies of $(notdir $<) $(CINFO) ...
	@$(CPP) -MM -MG -MT $@ -MT $(@:.d=.o) $(CPPFLAGS) $(INCLUDE) $< > $@

# Libraries
%.a :
	@$(ECHO) $(AR) $@
	@$(AR) $(ARFLAGS) $@ $^ > $(NULL)

%.so :
	@$(ECHO) $(LD) -shared $(OUTPUT_OPTION) $(LDINFO) $(notdir $^) $(LIBS)
	@$(LD) -shared $(LDFLAGS) $(OUTPUT_OPTION) $^ $(LDPATH) $(LIBS)

ifdef XMINGW32
.SUFFIXES : .exp .def .lib .dll
vpath %.exp $(OBJ)
$(OBJ)/%.exp : %.def
	@$(ECHO) $(DLLTOOL) -e $@ -d $(notdir $<)
	@$(DLLTOOL) -e $@ -d $<

#%.lib : %.def
#	@$(ECHO) $(DLLTOOL) -l $@ -d $(notdir $<)
#	@$(DLLTOOL) -l $@ -d $<

%.dll :
	@$(ECHO) $(LD) -shared $(OUTPUT_OPTION) $(LDINFO) $(notdir $^) $(LIBS)
	@$(LD) -shared $(LDFLAGS) $(OUTPUT_OPTION) $^ $(LDPATH) $(LIBS)

%.lib :
	@$(ECHO) $(LD) -shared -o $(basename $@).dll $(LDINFO) $(notdir $^) $(LIBS)
	@$(LD) -shared $(LDFLAGS) -Wl,--out-implib -Wl,$@ -o $(basename $@).dll $^ $(LDPATH) $(LIBS)
endif

# Linked executable
% :
	@$(ECHO) $(LD) $(OUTPUT_OPTION) $(LDINFO) $(notdir $^) $(LIBS)
	@$(LD) $(LDFLAGS) $(OUTPUT_OPTION) $^ $(LDPATH) $(LIBS)

# Strip off debugging symbols
define do-split
	@$(ECHO) $(STRIP) $@
	@$(STRIP) --only-keep-debug $< -o $@
	@$(STRIP) -s $<
	@$(OBJCOPY) --add-gnu-debuglink=$@ $<
endef
%.dbg : %
	$(do-split)
%.dbg : %.exe
	$(do-split)

# Archives
%.tar.gz :
	tar -c --dereference --numeric-owner $^ | gzip > $@

%.tar.bz2 :
	tar -c --dereference --numeric-owner $^ | bzip2 -zfc > $@

%.zip :
	zip -q $@ $^


########################
# Explicit rules for building objects go here.

.PHONY: all
all : $(TARGET)

.PHONY: FORCE
FORCE: ;

# Create the build dir
STAMP = $(OBJ)/stamp
$(STAMP) :
	@$(call md,$(@D))
	@$(call touch,$@)

.PHONY: syms
syms: $(basename $(TARGET)).dbg

.PHONY : suid
suid : $(TARGET)
ifndef DEBUG
ifndef DJGPP
ifndef XDJGPP
ifndef XMINGW32
ifndef CYGDIR
	sudo chown root:root $< && sudo chmod u+s $<
endif
endif
endif
endif
endif

.PHONY : debug
nodeps += debug
debug :
	@$(MAKE) -s DEBUG=1

.PHONY : debugcxx
nodeps += debugcxx
debugcxx :
	@$(MAKE) -s DEBUG=cxx

ifdef DJGPP
# standalone version linked with DPMI server
DJDIR ?= c:\djgpp
DJSTUB ?= $(DJDIR)\bin\CWSDSTUB.exe
.PHONY: standalone
standalone: $(TARGET) $(DJSTUB)
	@exe2coff $<
	copy /b $(subst /,\,$(DJSTUB)) + $(basename $<) $< > $(NULL)
	@-del $(basename $<)
endif

.PHONY: tar
nodeps += tar
tar : $(basename $(TARGET)).tar.bz2
ARCHIVE +=\
 $(wildcard [Mm]akefile)\
 $(wildcard *.mak)\
 $(wildcard *.bat)\
 $(wildcard *.c)\
 $(wildcard *.cpp)\
 $(wildcard *.cc)\
 $(wildcard *.h) \
 $(wildcard *.S)\
 $(wildcard *.s)
$(basename $(TARGET)).tar.bz2 : $(ARCHIVE)

.PHONY: zip
nodeps += zip
zip : $(basename $(TARGET)).zip
$(basename $(TARGET)).zip : $(ARCHIVE)

.PHONY: tidy
nodeps += tidy
tidy :
ifdef DJGPP
	@-$(call rm,$(OBJ)/*.o)
	@-$(call rm,$(OBJ)/*.s)
	@-$(call rm,$(OBJ)/*.d)
	@-$(call rm,$(OBJ)/*)
else
	@-$(call rm,*~)
endif
	@-$(call rd,$(OBJ))
	@-$(call rm,gmon.out)

.PHONY: clean
nodeps += clean
clean :
	@-$(call rm,$(TARGET))
	@-$(call rm,$(basename $(TARGET)).dbg)
clean : tidy


########################
# Dependencies:

APPOBJS = $(call src2o,$(SOURCES))
$(APPOBJS) : $(STAMP) $(MAKEFILE_LIST)

ifneq ($(TARGET),$(call exe,$(SOURCES)))
#ifneq ($(basename $(TARGET)),$(SOURCES))
 $(TARGET) : $(APPOBJS)
endif

ifdef PROFILE
 # Enabling profiling always rebuilds all objects
 $(APPOBJS) : FORCE
 ifeq ($(TARGET),$(call exe,$(SOURCES)))
 #ifeq ($(basename $(TARGET)),$(SOURCES))
  $(TARGET) : FORCE
 endif
endif

DEPENDS := $(call src2o,$(if $(SOURCES),$(SOURCES),$(basename $(TARGET))) $(DEPENDS))
DEPENDS := $(DEPENDS:.o=.d)
$(DEPENDS) : $(STAMP)

.PHONY: depends
nodeps += depends
depends: $(DEPENDS) ;
ifneq (,$(findstring depends,$(MAKECMDGOALS)))
 # make depends forces rebuild of DEPENDS
 $(DEPENDS): FORCE
else
 AUTODEPENDS ?= 0
 ifneq (1,$(AUTODEPENDS))
  # Don't make DEPENDS if they don't exist
  $(DEPENDS) : ;
 endif
endif

SRCS ?= $(foreach path,$(if $(SRCPATH),$(SRCPATH),.),\
 $(wildcard $(path)/*.c)\
 $(wildcard $(path)/*.cpp)\
 $(wildcard $(path)/*.cc)\
 $(wildcard $(path)/*.S)\
)
.PHONY: depend
nodeps += depend
depend: $(SRCS)
	makedepend -Y -p$(OBJ)/ $(CPPFLAGS) $(INCLUDE) $^ 2> $(NULL)

nodeps += clean-lib
#ifneq (,$(filter-out $(nodeps),$(if $(MAKECMDGOALS),$(MAKECMDGOALS),all)))
ifeq (,$(filter $(nodeps),$(MAKECMDGOALS)))
 -include $(DEPENDS)
endif
# End of file
