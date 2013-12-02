#!/bin/sh
#
# Copyright (c) 2000, 2004 Tama Communications Corporation
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Bless.sh: rewrite id's value of html for centralised cgi script.
#
# Usage:
#	% htags -fS		<- works well at generated place.
#	% mv HTML /var/obj	<- move to another place. It doesn't work.
#	% cd /var/obj/HTML
#	% sh bless.sh		<- OK. It will work well!
#
action=@action@
case $1 in
-v)	verbose=1;;
esac
id=`pwd`
for f in mains.html index.html search.html; do
	if [ -f $f ]; then
		sed -e "s!<\(input type='hidden' name='id' value\)='[^']*'\(.*\)>!<\1='$id'\2>!" \
		   -e "s!<\(form method='get' action\)='[^']*'\(.*\)>!<\1='$action'\2>!" $f > $f.new;
		if cmp $f $f.new >@null_device@; then
			rm -f $f.new
		else
			mv $f.new $f
			[ $verbose ] && echo "$f was blessed."
		fi
	fi
done
rm -f cgi-bin/global.cgi
