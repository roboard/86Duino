#! d:/djgpp/bin/perl
#
# Copyright (c) 1997, 2004, 2006 Tama Communications Corporation
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#------------------------------------------------------------------
# SORRY TO HAVE SURPRISED YOU!
# IF YOU SEE THIS UNREASONABLE FILE WHILE BROUSING, FORGET PLEASE.
# IF YOU ARE A ADMINISTRATOR OF THIS SITE, PLEASE SETUP HTTP SERVER
# SO THAT THIS SCRIPT CAN BE EXECUTED AS A CGI COMMAND. THANK YOU.
#------------------------------------------------------------------
print "Content-type: text/html\n\n";
print "@page_begin@\n";
print "@body_begin@\n";
$htmlbase = $ENV{'HTTP_REFERER'};
if (!$htmlbase) {
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Your browser doesn't send HTTP_REFERER.@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
$htmlbase =~ s!/[^\/]+$!!;		# remove file name.
$htmlbase =~ s!/defines$!!;
$htmlbase =~ s!/S$!!;
$htmlbase =~ s!/$!!;
if (! -x '@globalpath@') {
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Server side command not found. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
@pairs = split (/&/, $ENV{'QUERY_STRING'});
foreach $p (@pairs) {
	($name, $value) = split(/=/, $p);
	$value =~ tr/+/ /;
	$value =~ s/%([\dA-Fa-f][\dA-Fa-f])/pack("C", hex($1))/eg;
	$form{$name} = $value;
}
if ($form{'pattern'} eq '') {
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Pattern not specified. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
$pattern = $form{'pattern'};
$flag = '';
$words = 'definitions';
if ($form{'type'} eq 'reference') {
	$flag = 'r';
	$words = 'references';
} elsif ($form{'type'} eq 'symbol') {
	$flag = 's';
	$words = 'symbols';
} elsif ($form{'type'} eq 'path') {
	$flag = 'P';
	$words = 'paths';
} elsif ($form{'type'} eq 'grep') {
	$flag = 'g';
	$words = 'patterns';
} elsif ($form{'type'} eq 'idutils') {
	$flag = 'I';
	$words = 'patterns';
}
$iflag = '';
if ($form{'icase'}) {
	$iflag = 'i';
}
$oflag = '';
if ($form{'other'} && ($flag eq 'g' || $flag eq 'P')) {
	$oflag = 'o';
}
if ($form{'id'}) {
	chdir("$form{'id'}/cgi-bin");
	if ($?) {	
		print "@error_begin@Error@error_end@\n";
		print "@message_begin@Couldn't find tag directory in secure mode. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
		print "@body_end@\n";
		print "@page_end@\n";
		exit 0;
	}
}
chdir("../..");
if ($?) {	
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Couldn't change directory for $form{'type'} search. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
#
# Sanity check
# GTAGS and GPATH is indispensable file.
#
$missed = '';
if (! -f 'GTAGS') {
	$missed = 'GTAGS';
} elsif (! -f 'GPATH') {
	$missed = 'GPATH';
} elsif ($flag eq 'r' && ! -f 'GRTAGS') {
	$missed = 'GRTAGS';
} elsif ($flag eq 's' && ! -f 'GSYMS') {
	$missed = 'GSYMS';
} elsif ($flag eq 'I' && ! -f 'ID') {
	$missed = 'ID';
}
if ($missed) {
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Tag file ($missed) not found. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
local(%ctab) = ('&', '&amp;', '<', '&lt;', '>', '&gt;');
#
# fork and exec global(1) to avoid command substitutions in $pattern.
# The --result=ctags-xid print the file id of the path at the head
# of each line.
#
$flags = $flag . $iflag . $oflag;
if ($flags eq '') {
	open(PIPE, "-|") || exec '@globalpath@', '--result=ctags-xid', '-e', $pattern;
} else {
	open(PIPE, "-|") || exec '@globalpath@', '--result=ctags-xid', '-'.$flags, '-e', $pattern;
}
if ($?) {	
	print "@error_begin@Error@error_end@\n";
	print "@message_begin@Cannot execute global. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
	print "@body_end@\n";
	print "@page_end@\n";
	exit 0;
}
$pattern =~ s/([&<>])/$ctab{$1}/ge;
print "@title_begin@" . $pattern . "@title_end@\n";
print "Following $words are matched to above pattern.@hr@\n";
$cnt = 0;
local($tag, $lno, $filename);
print "@verbatim_begin@\n";
#
# Input format:
#
# fid tag   lno filename
# ---------------------------------------------
# 100 main  32 ./main.c main(argc, argv)
#
while (<PIPE>) {
	$cnt++;
	($fid, $tag, $lno, $filename) = split;
	s/^[0-9]+[ \t]+//;
	s/([&<>])/$ctab{$1}/ge;
	s!(^[^ \t]+)!<a href='$htmlbase/S/$fid.@HTML@#L$lno'>$1<\/a>!;
	print;
}
close(PIPE);
print "@verbatim_end@\n";
if ($cnt == 0) {
	print "@message_begin@Pattern not found. <a href='$htmlbase/mains.@normal_suffix@'>[return]</a>@message_end@\n";
}
print "@hr@\n";
print "@body_end@\n";
print "@page_end@\n";
exit 0;
#------------------------------------------------------------------
# SORRY TO HAVE SURPRISED YOU!
# IF YOU SEE THIS UNREASONABLE FILE WHILE BROUSING, FORGET PLEASE.
# IF YOU ARE A ADMINISTRATOR OF THIS SITE, PLEASE SETUP HTTP SERVER
# SO THAT THIS SCRIPT CAN BE EXECUTED AS A CGI COMMAND. THANK YOU.
#------------------------------------------------------------------
