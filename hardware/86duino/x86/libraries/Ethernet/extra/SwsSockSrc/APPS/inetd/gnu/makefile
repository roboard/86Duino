#!/usr/bin/make
# build inetd
#
# REVISION HISTORY
#
#  No  Date       By   Reason
# -------------------------------------------------------------------------
#  02  14 May 09  lvr  Revised to use makefile
#  01  12 Aug 06  lvr  Creation
# -------------------------------------------------------------------------
#
# Platforms
#----------
# Linux (gcc 4.1 and 4.2) builds and runs OK.
# XDJGPP 2.04 (gcc 4.3) builds and runs OK.
# XMINGW32 (gcc 4.2) builds OK.

TARGET = inetd
SOURCES := $(TARGET)
BSDSOURCES := $(TARGET)
DAEMON ?= 1
ifneq (0,$(DAEMON))
 NOTANSISOURCES := $(TARGET)
endif
SRCPATH = ..
INCPATH = ..

UPNP ?= 1
ifneq (0,$(UPNP))
SOURCES += sws_upnp
BSDSOURCES += sws_upnp
CPPFLAGS += -DOPT_UPNP=$(UPNP)
endif

FTPD ?= 1
ifneq (0,$(FTPD))
SOURCES += ftpd
BSDSOURCES += ftpd
POSIXSOURCES += ftpd
NOTANSISOURCES += ftpd
SRCPATH += ../../ftpd
INCPATH += ../../ftpd
endif

HTTPD ?= 1
ifneq (0,$(HTTPD))
SOURCES += httpd httpfile
BSDSOURCES += httpd
POSIXSOURCES += httpfile
NOTANSISOURCES += httpfile
ifeq (,$(findstring sws_ba64, $(SOURCES)))
SOURCES += sws_ba64
endif
ifeq (,$(findstring sws_time, $(SOURCES)))
SOURCES += sws_time
endif
SRCPATH += ../../httpd
INCPATH += ../../httpd
PROXY ?= 1
ifneq (0,$(PROXY))
SOURCES += httpprxy
BSDSOURCES += httpprxy
endif
CGI ?= 1
ifneq (0,$(CGI))
SOURCES += httpcgi
BSDSOURCES += httpcgi
POSIXSOURCES += httpcgi
NOTANSISOURCES += httpcgi
endif
V4L ?= 0
ifneq (0,$(V4L))
SOURCES += httpv4l
BSDSOURCES += httpv4l
LIBS += -ljpeg
endif
endif

TELNETD ?= 1
ifneq (0,$(TELNETD))
SOURCES += telnetd
BSDSOURCES += telnetd
POSIXSOURCES += telnetd
NOTANSISOURCES += telnetd
SRCPATH += ../../telnetd
INCPATH += ../../telnetd
endif

SMTPD ?= 1
ifneq (0,$(SMTPD))
SOURCES += smtpd
BSDSOURCES += smtpd
POSIXSOURCES += smtpd
NOTANSISOURCES += smtpd
ifeq (,$(findstring sws_ba64, $(SOURCES)))
SOURCES += sws_ba64
endif
ifeq (,$(findstring sws_time, $(SOURCES)))
SOURCES += sws_time
endif
SRCPATH += ../../smtpd
INCPATH += ../../smtpd
SMTP_FORWARD ?= 1
ifneq (0,$(SMTP_FORWARD))
SOURCES += sendmail
BSDSOURCES += sendmail
SRCPATH += ../../sendmail
INCPATH += ../../sendmail
endif
endif

POPD ?= 1
ifneq (0,$(POPD))
SOURCES += popd popmsg
ifeq (,$(findstring sws_ba64, $(SOURCES)))
SOURCES += sws_ba64
endif
BSDSOURCES += popd
POSIXSOURCES += popmsg
NOTANSISOURCES += popmsg
SRCPATH += ../../popd
INCPATH += ../../popd
CRAM_MD5 ?= 1
endif

ECHOD ?= 1
ifneq (0,$(ECHOD))
SOURCES += echod
BSDSOURCES += echod
SRCPATH += ../../echos
INCPATH += ../../echos
endif

IDENTD ?= 1
ifneq (0,$(IDENTD))
SOURCES += identd
BSDSOURCES += identd
SRCPATH += ../../identd
INCPATH += ../../identd
ifdef XMINGW32
 LIBS += -liphlpapi -lmpr
endif
endif

NETBIOSD ?= 0
ifneq (0,$(NETBIOSD))
SOURCES += netbiosd
BSDSOURCES += netbiosd
SRCPATH += ../../netbiosd
INCPATH += ../../netbiosd
endif

# Ensure sws_dns is built with native networking
SOURCES += sws_dns

# Swssock
# SwsSock native threads
THREADS = 2
SWSSOCK = 3
ROOT_DIR = ../../..
include $(ROOT_DIR)/LIBS/gnu/makefile

$(call obj-var,inetd,CPPFLAGS +=\
 -DPACKET_DUMP=1\
 -DINETD_FTP=$(FTPD)\
 -DINETD_HTTP=$(HTTPD) -DHTTP_PROXY=$(PROXY) -DHTTP_CGI=$(CGI) -DHTTP_V4L=$(V4L)\
 -DINETD_TELNET=$(TELNETD)\
 -DINETD_SMTP=$(SMTPD)\
 -DINETD_POP=$(POPD)\
 -DINETD_ECHO=$(ECHOD) -DINETD_DISCARD=$(ECHOD)\
 -DINETD_IDENT=$(IDENTD)\
 -DINETD_NETBIOS=$(NETBIOSD)\
 -DINETD_DAEMON=$(DAEMON)\
)
$(call obj-var,telnetd,CPPFLAGS += -DTELNETD_EXT=2) #1= use system(), 2= use POSIX popen
$(call obj-var,smtpd,CPPFLAGS += -DOPT_FORWARD=$(SMTP_FORWARD))
$(call obj-var,popd,CPPFLAGS += -DOPT_CRAM_MD5=$(CRAM_MD5))

$(call obj-var,ftpd,WARN += -Wno-shadow)
$(call obj-var,netbiosd,WARN += -Wno-strict-aliasing)
WARN += -Wno-cast-qual


# Linux daemon install/remove
INSTALL = install -c
ifdef DEBUG
UPDATE_RC = @echo update-rc.d
ADDUSER = @echo adduser
DELUSER = @echo deluser
CHOWN = @echo chown
else
UPDATE_RC = update-rc.d
ADDUSER = adduser --quiet
DELUSER = deluser --quiet
CHOWN = chown
endif

ifdef DEBUG
DESTDIR = ./install
else
DESTDIR =
endif
SBINDIR := $(DESTDIR)/usr/local/sbin
INITDIR := $(DESTDIR)/etc/init.d
DAEMONX := sws_$(TARGET)
CFGDIR := $(DESTDIR)/etc/$(DAEMONX)
VARDIR := $(DESTDIR)/var/lib/$(DAEMONX)
LOGRDIR := $(DESTDIR)/etc/logrotate.d
LOGDIR := $(DESTDIR)/var/log
ADMGRP := adm

.PHONY : daemon
daemon:
	@$(MAKE) -s DAEMON=1 V4L=1

# NB to create a log-file after setuid() need write perm in /var/log
.PHONY : permissions
permissions:
	chmod g+w $(LOGDIR)
	chgrp $(ADMGRP) $(LOGDIR)

.PHONY : username
username:
	$(ADDUSER) --system --group --home $(VARDIR) --no-create-home $(DAEMONX)
	$(ADDUSER) $(DAEMONX) $(ADMGRP)

$(CFGDIR)/$(DAEMONX).ini: $(DAEMONX).ini
	$(call md,-m 755 $(CFGDIR))
ifdef DEBUG
	$(INSTALL) -m 640 $< $@
else
	$(INSTALL) -m 640 -g $(DAEMONX) $< $@
endif

.PHONY : install
install: daemon username $(CFGDIR)/$(DAEMONX).ini
	$(call md,-m 755 $(SBINDIR))
	$(INSTALL) -m 700 $(TARGET) $(SBINDIR)/$(DAEMONX)
	$(call md,-m 755 $(LOGRDIR))
	$(INSTALL) -m 644 $(DAEMONX).logrotate $(LOGRDIR)/$(DAEMONX)
	$(call md,-m 755 $(INITDIR))
	$(INSTALL) $(DAEMONX).d $(INITDIR)/
	$(call md,-m 755 $(VARDIR)/inbox)
	$(call md,-m 755 $(VARDIR)/ftp)
	$(call md,-m 755 $(VARDIR)/www)
	$(CHOWN) -R $(DAEMONX):$(DAEMONX) $(VARDIR)
	$(UPDATE_RC) $(DAEMONX).d defaults
	$(INITDIR)/$(DAEMONX).d start

.PHONY : remove
remove :
	-$(INITDIR)/$(DAEMONX).d stop
	-$(call rm,$(INITDIR)/$(DAEMONX).d)
	-$(UPDATE_RC) -f $(DAEMONX).d remove
	-$(DELUSER) --system $(DAEMONX)
	-$(call rm,$(SBINDIR)/$(DAEMONX))
	-$(call rm,$(LOGRDIR)/$(DAEMONX))

.PHONY : uninstall
uninstall: remove

.PHONY : removeall
removeall : remove
	-$(call rd,$(VARDIR))
	-$(call rd,$(CFGDIR))
ifdef DEBUG
	-$(call rd,$(DESTDIR))
else
	-$(call rm,$(LOGDIR)/$(DAEMONX).*)
endif

.PHONY : update
update : daemon
	@$(MAKE) -s remove
	@$(MAKE) -s install
