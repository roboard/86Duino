#! /bin/sh
### BEGIN INIT INFO
# Provides:          common internet services
# Required-Start:    $local_fs $remote_fs
# Required-Stop:     $local_fs $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: sws_inetd initscript
# Description:       This file should be placed in /etc/init.d.
### END INIT INFO
#
# Author:	Lawrence Rust <lvr@softsystem.co.uk>.
#
# Version:	@(#)sws_inet  1.0-2  11-Nov-2009  lvr@softsystem.co.uk
#

set -e

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DESC="Internet services"
NAME=sws_inetd
DAEMON=/usr/local/sbin/$NAME
PIDFILE=/var/run/$NAME.pid
DAEMON_ARGS="-d $NAME -o /var/log/$NAME.log -c /etc/$NAME/$NAME.ini -p $PIDFILE"
SCRIPTNAME=/etc/init.d/$NAME.d

# Gracefully exit if the package has been removed.
test -x $DAEMON || exit 0

# Get lsb functions
. /lib/lsb/init-functions

# Read config file if it is present.
if [ -r /etc/default/$NAME ]
then
. /etc/default/$NAME
fi

#
#	Function that starts the daemon/service.
#
d_start() {
	start-stop-daemon --start --quiet \
		--pidfile $PIDFILE --exec $DAEMON -- $DAEMON_ARGS \
		|| echo -n " already running"
}

#
#	Function that stops the daemon/service.
#
d_stop() {
	start-stop-daemon --stop --quiet \
		--pidfile $PIDFILE --signal 2 --retry 2 \
		|| echo -n " not running"
}

#
#	Function that sends a SIGHUP to the daemon/service.
#
d_reload() {
	start-stop-daemon --stop --quiet \
		--pidfile $PIDFILE --signal 1
}

case "$1" in
  start)
  log_begin_msg "Starting $DESC: $NAME..."
	d_start
  log_end_msg $?
	;;
  stop)
  log_begin_msg "Stopping $DESC: $NAME..."
	d_stop
  log_end_msg $?
	;;
  reload)
	#
	#	If the daemon can reload its configuration without
	#	restarting (for example, when it is sent a SIGHUP),
	#	then implement that here.
	#
	#	If the daemon responds to changes in its config file
	#	directly anyway, make this an "exit 0".
	#
	log_begin_msg "Reloading $DESC: $NAME..."
	d_reload
  log_end_msg $?
  ;;
  restart|force-reload)
	#
	#	If the "reload" option is implemented, move the "force-reload"
	#	option to the "reload" entry above. If not, "force-reload" is
	#	just the same as "restart".
	#
  log_begin_msg "Restarting $DESC: $NAME..."
	d_stop
	# One second might not be time enough for a daemon to stop,
	# if this happens, d_start will fail (and dpkg will break if
	# the package is being upgraded). Change the timeout if needed
	# be, or change d_stop to have start-stop-daemon use --retry.
	# Notice that using --retry slows down the shutdown process somewhat.
	sleep 1
	d_start
  log_end_msg $?
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop|restart|force-reload}" >&2
	exit 3
	;;
esac

exit 0
