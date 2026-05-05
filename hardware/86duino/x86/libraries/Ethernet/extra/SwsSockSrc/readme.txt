SWSSOCK - TCP/IP NETWORK COMMUNICATIONS LIBRARY
===============================================

Author: Lawrence Rust (lvr@softsystem.co.uk)
Last Modified: 05-Feb-11

SwsSock is a cross platform networking library compatible with the Single Unix
Specification v3 and Winsock2 sockets providing a fully featured TCP/IP protocol
stack and drivers for common Ethernet and serial adapters.

The package consists of sources, C header files, example applications and
projects or makefiles for the following target/compiler combinations:

- Linux, gcc 3.x or later.

- Win32, Mingw or Microsoft Visual C++ 6.0 or later.

- 32-bit DOS, DJGPP (version 2.03 or later) or OpenWatcom.

- 16-bit DOS, OpenWatcom or Microsoft Visual C++ 1.52c.

The SwsSock library is shareware; that is it may be freely used by any
individual for personal and private use or for their business without any
restriction or charge.  Companies, organisations or governments may not use it
without a license.  In no case may SwsSock be re-sold or used in a system to
deprive anyone of life, liberty or livelihood.

Copyright (C)2005-2011 Software Systems Consultants.
Web: www.softsystem.co.uk
Tel: +33 5 49 68 72 47
e-mail: sales@softsystem.co.uk


FEATURES
========

- Single Unix Specification v3 and Winsock2 sockets AOI.
- TCP.  Supports slow start, congestion control (RFC2581), Jacobson RTT
  estimation, fast retransmit etc...
- IP version 4.  Supports IP options, fragmentation and reassembly, fast
  table driven routing & cache, multi-homed routing.
- UDP.
- Raw sockets.
- ICMP and Echo server.
- IGMP version 2 and multicasting.
- Ethernet - DIX and 802.3 frame formats.
- ARP.
- Point to point link manager featuring 'dial on demand'.
- Hayes modem support.
- 'MSdirect' modem support for Microsoft direct cable connections.
- PPP and SLIP serial links.
- CHAP and PAP authentication.
- Van Jacobsen TCP/IP header compression.
- DNS client.
- DHCP client.
- IP, PPP and Ethernet packet dumpers.
- Threads.  Mapped to native threads on appropriate platforms, otherwise a
  co-operative (non-preemptive) implementation is provided.
- Timers.  Low overhead with millisecond resolution.
- Low overhead, minimum copy buffering.
- Fully asynchronous design.
- Simple configuration from text file.
- Platform independent.
- Easily portable design written entirely in ANSI C89.
- Easily customised - simple interfaces to Ethernet and serial drivers.
- Ideal for embedded systems.
- Small and fast; complete HTTP 1.1 web server in 87 KBytes.


REQUIREMENTS
============

- A PC running Linux (2.6.x), Windows (Vista, XP, 2000, NT4, Me, 98) or MS-DOS.

- 10 MB of free disk space.

- A Linux or Windows supported TCP/IP network or an Ethernet card and packet
  driver for DOS.

- A suitable C compiler.  ANSI C89 compatible.

- For native DOS you need a standard 'packet driver' for your Ethernet adapter
  (see Crynwr Software www.crynwr.com) for examples.  You will need to add this
  driver to your config.sys or autoexec.bat file and reboot.

- The DOS applications can also be run under Windows in which case you will need
  our SwsVpkt virtual packet driver.  See www.softsystem.co.uk/products/swsvpkt.
  Each session provides a standalone TCP/IP implementation.  Sessions
  may be given identical or different IP addresses dependent upon the local
  sws_sock.ini file contents.


LIMITATIONS
===========

The Windows shared library provides a full implementation of SwsSock but
requires the SwsVpkt packet driver to access a LAN.  The SwsSock DLL provides a
functional subset of Winsock2.  All standard BSD sockets calls are supported
but there is a restricted set of WSAxxx Microsoft specific functions and
associated functionality.  In many cases, an application linked with ws2_32.dll
can run quite happily using the (renamed) SwsSock DLL.

The 16-bit DOS library must be compiled as 'large model' assuming SS != DS.
Threaded programs must also be compiled assuming SS != DS.


INSTALLATION
============

DOS/Windows: If you haven't already done so, download the virtual packet driver,
SwsVpkt, from http://www.softsystem.co.uk/download/swsvpkt/SwsVpkt1005.zip and
install it.

If you have a self-extracting archive file (.exe) then double click it to
expand the files into an empty folder.  If you have a ZIP archive then use
WinZip or similar to extract the files into an empty folder.

The example applications should now be built (see 'Building the examples' below)
to ensure correct installation.


REMOVAL
=======

Simply delete the folder created during installation.



FILE LAYOUT
===========

In the top level directory:

apps.txt      Description of the applications and quick-start guide.
c_api.txt     Description of the C programming interface.
config.txt    Description of the sws_sock.ini runtime configuration file.
readme.txt    This file
src.txt       Description of the library architecture
sws_sock.ini  The runtime configuration file.


In the APPS directory:

Batch files to build the applications.
A sub-directory for each example.


In the APPS\<Example> directory:

C source files and headers for the example.
A sub-directory containing make or project files for each target.


In the INCLUDE directory:

C header files defining the public interface to each module.

sws_sock.h   C header containing declarations of the 'sockets' API.
sws_net.h    C header bringing in network headers for each target.


In the LIBS directory:

A sub-directory for each target containing a library and the file config.h which was used
to specify library build options.

sws_cfg.c    C source to read the sws_sock.ini file.
sws_dmpe.c   C source to the Ethernet packet dumper.
sws_dmpi.c   C source to the IP packet dumper.
sws_dmpp.c   C source to the PPP packet dumper.
sws_dump.c   C source to the packet dumper.
sws_dns.c    C source to the DNS query utility.
sws_upnp.c   C source to the Universal Plug'n'Play utility.


EXAMPLES
========

There are example, console mode, programs provided in the 'APPS' folder:

- Echos.
  A combined UDP/TCP Echo (RFC862) server and client. Makefiles/projects are
  provided for Cygwin, Djgpp, MSVC++1.52c, MSVC++ 6 and MSVC++ 6 using
  Winsock2.

- Httpd.
  An HTTP 1.1 web server with file system module.  The server can be
  extended by registering 'action modules' to handle specified URI branches.
  Makefiles/projects are provided for Cygwin, Djgpp, MSVC++1.52c, MSVC++ 6 and
  MSVC++ 6 using Winsock2.

- Inetd.
  An example of using SWS_Thread to create a multi-threaded server combining
  Echod and Httpd.  Makefiles/projects are provided for Djgpp, MSVC++1.52c and
  MSVC++ 6.

- Ping.
  An ICMP echo client.  This example demonstrates using 'raw' sockets.
  Makefiles/projects are provided for Cygwin, Djgpp, MSVC++1.52c, MSVC++ 6 and
  MSVC++ 6 using Winsock2.  NB the Winsock2 version will not work on Windows XP
  service pack 2 due to Microsoft disabling raw socket support in this service
  pack.

- Pktdump.
  An example of using the Ethernet and IP packet dumper API.
  Makefiles/projects are provided for Djgpp, MSVC++1.52c and MSVC++ 6.

- And many more...


Building the examples
----------------------

The examples can be built by double clicking the batch file:

* balinux.sh.
  Builds Linux native programs.  You will need to enter your sudo password to
  build some suid programs like ping.

* buildalldjgpp.bat.
  Builds 32-bit DOS programs with DJGPP 2.03 or later. You may need to edit
  makedj.bat to setup paths.

* buildallmsvc1.bat.
  Builds 16-bit DOS programs with MSVC 1.52c.  You may need to edit makevc1.bat
  to setup paths.

* buildallmsvc6.bat.
  Builds Win32 programs using the SwsSock DLL with MS VC++ 6. You may need to
  edit makevcs.bat to setup paths.

* buildallmsvc6ws2.bat.
  Builds Win32 programs using the Winsock2 DLL with MS VC++ 6. You may need to
  edit makevc.bat to setup paths.

* buildallcygwin.bat.
  Builds Win32 native programs with the Cygwin compiler. You may need to edit
  makecygw.bat to setup paths.

It is also possible to build the 16-bit DOS programs with BorlandC++ 3.1.


Running the examples
--------------------

For convenience all programs of a given type should be copied to a separate
runtime folder.

Copy the file sws_sock.ini (found in the top level folder where SwsSock was
installed) to the runtime folder.

Open a command line session and cd to the runtime folder.

Help for each program can be obtained by entering a command line of -? .

See apps.txt for help on running the applications.


CONFIGURATION
=============

All configuration data is contained in the text file sws_sock.ini. This file
must be placed in the same directory as the executable program.  Please refer
to config.txt for information on the contents of this file.


PROBLEMS
========

SwsSock is designed to be compatible with all recent Windows systems.  The
following limitations are known:

- The DPMI server that ships with Windows NT4 service pack 6 has a problem with
  DJGPP applications.  This manifests as a SIGSEGV abort when a frame is
  received.  There is no known workaround.

If you should have other problems, issues or for technical support contact
sales@softsystem.co.uk or phone us on +33 5 49 72 79 63.


WHO ARE SOFTWARE SYSTEMS?
=========================

Software Systems provide software development services and consultancy.  Our
current products include:

 SwsSock TCP/IP communications library
 SwsVpkt virtual packet driver
 MailCenturion
 NetCenturion
 SoundFX
 Masm2djg

Contacts:

Web: www.softsystem.co.uk
e-mail: sales@softsystem.co.uk


WARRANTY
========

The SwsSock evaluation is supplied 'as is' with absolutely no warranty given or
implied.  The use of SwsSock is entirely at the risk of the end user.  Software
Systems will not be held responsible for any loss or damage, however caused,
resulting from the use of SwsSock.


Release History
===============
2.0.0.13 19-Feb-11 httpd: Fix a bug with HEAD requests not closing the
                     connection until idle timeout.
                   ftpd: Fix a bug in passive connections hanging in accept.
                     Close & re-use oldest connection if all connections in
                     use.
                     Limit directory name lengths.
                     Add more log output.
                   smtpd: Limit login attempts and lockout abusive IP addresses.
2.0.0.12 01-Feb-11 ftpd: Limit login attempts and lockout abusive IP addresses
                   smtpd: Canonical IP address matching ignores ls 8-bits.
2.0.0.11 29-Aug-10 httpd: Fix null dereference after unexpected disconnect.
                   smtpd: Better spam rejection.
                   Add swsinfo app to print configuration to stdout.
                   Add SWS_CfgGetName.
                   Linux lib initialisation fix DNS name server setup.
2.0.0.10 16-Jul-10 Httpd security fix for alias expansion.
                   Popd and smtpd bug fixes.
                   Loganaly - Add -s option for simpler pattern matching.
2.0.0.9 13-Mar-10 Popd add auth plain, login and cram-md5 authentication.
                  Inetd daemon code supports groups ids.
                    Sighup caught and logfile reopened for logrotate.
                    Add -c and -p options to specify config and pid file.
                  Numerous apps have logfile tidy up.
                  Httpd refactor code to improve maintainability.
                    Security fox for spaces in CGI path.
                    Bugfix for idle timeouts.
                  Realip -r mode buffer overrun bugfix
                  httpd, smtpd, ftpd, pop3d & telnetd report unsuccessful client
                    ops as warnings as opposed to info.
                  ip.c additional runtime checks on header length as suggsted by
                    draft-ietf-opsec-ip-security-02.txt
                  Add MD5 digest to library.
                  Add base64 encoding/decoding to library
                  Add wild card string compare to library.
                  Add RFC822 time/date functions to the library.
2.0.0.8 05-Mar-10 Add support for restricted realms to httpd CGI.
                  PHP (php-cgi) support for httpd CGI.
                  Recvmail & sendmail extensions to support httpd CGI.
                  Inetd catches SIGHUP to re-open logfile, supporting log-rotate.
                  Inetd security fix to correctly drop root group membership.
2.0.0.7 21-Feb-10 Dns: host resolver o/p made compatible with direct dns method
                  ftpd/httpd/identd/popd/smtpd/telnetd: log verbosity from cfg file
                  httpd: URI alias/re-direct lookup made sequential
                    CGI bug fixes
                  popd: max login attemps set to 2
                    bug fix unexpected disconnect crash
                  recvmail mods to support webmail cgi
                  smtpd: add banned senders, mail names, dynamic names, static names
                    bug fix banned hostnames localhost
                  sws_cfg.c: close unneeded config file
2.0.0.6 05-Feb-10 Httpd fix bug in CGI directory handling.
                  Echod/Httpd fix error handling on startup.
                  Inetd catch sighup and reopen logfile.
                  Httpcgi add REMOTE_PORT variable.
                  Loganaly add stats for protocol, mailopen, error & warn.
                  All servers: more consistent error/warning messages
                  Extended config.txt with app config info.
                  Dns & whois support for CGI output
                  Ping v2.06 support CGI.
                  mcast, mping, realip, syslogd, syslogr, upnp help to stderr.
2.0.0.5 29-Jan-10 Httpd v2.1.2 - new URI alias list with wildcards.
                    - improved common authentication system allows each 'realm'
                      to have its own folder.
                    - cache common ini file settings.
                    - CGI has more log output.
                    - New user specified MIME types.
                    - bug fix: CGI ( Windows/DOS environment variables).
                  Ftpd v1.0.5 - added 'x' user permission (cwd)
                  Smtpd v1.0.5 - New optional script after data received.
                    - cache common ini file settings.
                  Telnetd v2.0.4 - added IP address restriction to accounts.
                  Loganaly - more filters, factored common stats code.
                  SWS_CfgSetName parses the ini file and sets up a section cache
                    giving much reduced library startup time.
                  IP subnet mask now set automatically if not specified.
2.0.0.4 18-Jan-10 Httpd - new common authentication model for all modules
                    giving reduced code size and  better performance.
                  Inetd - new, simplified task definition.
                    Bug fix race condition during daemon startup.
                  Loganaly - new stats for HTTP user-agent, platform and referrer.
                  Sendmail - better diagnostics if mail server login fails.
                  Smtpd< - new config options cache for better performance.
                  Build scripts updated.
2.0.0.3 11-Jan-10 Httpd - URI redirection list now uses faster binary search.
                    - More detailed logging.
                    - Proxy supports an IP blacklist and content compression.
                    - CGI redirection bugs fixed.
                  Loganaly - add stats for top HTTP URIs
2.0.0.2 07-Jan-10 Httpd - Minor tweaks to improve proxy performance and logging.
                  Loganaly - track changes in httpd log output.
2.0.0.1 06-Jan-10 Httpd - better CGI performance.
                    - Increased security for put and delete methods.
                    - Improved logging.
2.0.0.0 03-Jan-10 Open source distribution.
                  Common build system for Linux, DJGPP, MinGW and Cygwin.
                  OpenWatcom support.
                  IGMPv3, ECN, SACK support.
                  New apps: DNS client, echo client/server, FTP client,
                    FTP server, ident server, log analyser, multicast ping
                    client/server, POP3 client & server, realIP, SMTP client &
                    server, discard client/server, syslogd, telnet server,
                    TFTP client, UPnP client, whois
                  httpd adds CGI, UPnP, proxy, authorisation and Video4Linux
                  inetd adds Linux daemon.
                  ping supports raw sockets & broadcast.
                  DJGPP fixes atomic ops, log files flushed with fsync.
                  Bug fixes for big-endian systems,
                  P2P supports bi-dir authentication.
1.0.0.4 18-Oct-05 Add multicast capability to raw IP sockets.
                  Default IP transmit queue length increased from 32 to 64.
                  Bug fixes: TCP fast retransmit, multicast and IP reassembly.
1.0.0.3 13-Sep-05 Bug fixes: TCP retransmit, DPC queue interrupt safety.
1.0.0.2 07-Sep-05 Added modem support and PPP.
1.0.0.1 24-Jun-05 Initial release.
