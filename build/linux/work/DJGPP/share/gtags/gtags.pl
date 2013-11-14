#
# Gtags.pl --- Global facility for Nvi-1.81
#
# Copyright (c) 2001, 2002 Tama Communications Corporation
#
# This file is part of GNU GLOBAL.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# usage:
#
#	nvi editor			command line
#	-----------------------------------------------------------
#	:perl tag qw(main)		global -x main
#	:perl tag qw(-T main)		global -Tx main
#	:perl tag qw(-r main)		global -rx main
#	:perl tag qw(-sl main)		global -slx main
#	:perl tag qw(-gi main)		global -gix main
#	:perl tag qw(-I main)		global -Ix main
#	:perl tag qw(-P file)		global -Px file
#	:perl tag qw(-f %)		global -fx <current file>
#	:perl gozilla			gozilla +<current line> <current file>
#
# Please type ':perl help[ENTER]' for help.
#
# Suggested .nexrc: (If you have gtags.pl in ${HOME}/perl.)
#
# perl use lib "$ENV{HOME}/perl"
# perl require 'gtags.pl'
# map ^P :tagprev^M
# map ^N :tagnext^M
# map ^] :perl tag^M
# map ^G :perl gozilla^M
# ab gtag perl tag qw(
# ab gta perl tag qw(
# ab gt perl tag qw(
# ab gozill perl gozilla
# ab gozil perl gozilla
# ab gozi perl gozilla
# ab goz perl gozilla

#
# You need this version of GLOBAL.
#
$support_version = 4.3;
#
# command existent check.
#
$w32 = ($^O =~ /^(ms)?(dos|win(32|nt))/i) ? 1 : 0;
$pathsep = ($w32) ? ';' : ':';
$command = '';
foreach (split(/$pathsep/, $ENV{'PATH'})) {
    if (-x "$_/global") {
        $command = "$_/global";
        last;
    }
}
#
# version check of global(1).
#
$notfound = '';
if ($command) {
    open(TAGS, "$command --version |");
    $_ = <TAGS>;
    chop($_);
    $version = $_;
    close(TAGS);
    $version =~ s/(\d+\.\d+)(\.\d+)?/\1/;
    if ($? != 0 || $version < $support_version) {
        $notfound = "Your global(1) seems to be older version.";
    }
} else {
    $notfound = "Global(1) not found.";
}
if ($notfound) {
    $notfound .= " You need GLOBAL-${support_version} or the later.";
}

sub main::tag {
    my $tagq, $tag, $flag;

    if ($notfound) {
        $curscr->Msg($notfound);
	return;
    }
    $flag = '';
    while ($_[0] =~ /^-\w+$/) {
        if ($_[0] =~ /^-([fgIPilrsTx]+)$/) {
            $flag .= $1;
            shift;
        } else {
            $curscr->Msg("'$_[0]' not acceptable. Please type ':perl help[ENTER]'.");
            return;
        }
    }
    if ($flag =~ /r/ && $flag =~ /s/) {
        $curscr->Msg("both of -s and -r are not allowed.");
        return;
    }
    if ($_[0]) {
        $tag = $_[0];
	#
	# replace '%' with current file name.
	#
	if ($tag =~ /%/) {
            $path = $curscr->GetFileName();
	    $tag =~ s/%/$path/;
	} else {
	}
    } else {
        #
        # get current position.
        #
        my($lineno, $column) = $curscr->GetCursor();
        my($line) = $curscr->GetLine($lineno);
	my($length) = length($line);
        #
        # extract the first word as a tag.
        #
	my($offset) = $column;
	while ($offset > 0 && substr($line, $offset, 1) =~ /^\w/) {
	    $offset--;
	}
	while ($offset < $length && substr($line, $offset, 1) !~ /^\w/) {
	    $offset++;
	}
	my($subline) = substr($line, $offset);
        ($tag, $blace) = ($subline =~ /(\w+)\s*(\(?)/);
        if (!$tag) {
            $curscr->Msg("tag not found in current position.");
            return;
        }
        #
        # decide flag value.
        #
	$flag = 's';
        if ($offset == 0) {
	    if ($blace) {
                $flag = 'r';		# maybe function definition.
	    }
        } else {
	    if ($line =~ /^#\s*define\s+(\w+)\(/ && $1 eq $tag) {
                $flag = 'r';		# maybe macro definition.
	    } elsif ($line =~ /^(ENTRY|ALTENTRY|NENTRY)\((\w+)\)$/ && $2 eq $tag) {
                $flag = 'r';		# maybe assember function definition.
	    } elsif ($blace) {
                $flag = 'x';		# maybe function reference.
	    }
        }
    }
    open(TAGS, "$command -xq$flag '$tag' |");
    $tagq = undef;
    while(<TAGS>) {
        my ($name, $lno, $path, $rest);
        if ((($name, $lno, $path, $rest) = /^([^ \t]+)[ \t]+(\d+)[ \t]+([^ \t]+)(.*)$/) >= 2) {
	    if (!$tagq) {
		$tagq = $curscr->TagQ($tag);
	    }
           $tagq->Add($path, $lno, '');
        }
    } 
    close(TAGS);
    $status = $?;
    $status = $status / 256;
    if ($status == 0) {
        if (!$tagq) {
	    $curscr->Msg("$tag: tag not found.");
        } else {
	    $tagq->Push();
        }
    } elsif ($status == 1) {
        $curscr->Msg("Global(1) failed. Please test it out of nvi.");
    } elsif ($status == 2) {
        $curscr->Msg("Usage error. Please type ':perl help[ENTER]'.");
    } elsif ($status == 3) {
        $curscr->Msg("GTAGS not found.");
    } else {
        $curscr->Msg("Unknown error.");
    }
}
sub main::gozilla {
    if ($notfound) {
        $curscr->Msg($notfound);
	return;
    }
    my($filename) = $curscr->GetFileName();
    my($lineno, $column) = $curscr->GetCursor();
    system("gozilla +$lineno $filename");
}
sub main::help {
    $help = <<END_OF_HELP;
Commands:
-f files: Print all function definitions in the files.
-g pattern: Print all lines which match to the pattern.
-I pattern: Print all lines which match to the pattern.
-P pattern: Print the path which match to the pattern.
Options:
-i: ignore case distinctions in pattern.
-l: Print just objects which exist under the current directory.
-r: Print the locations of object references.
-s: Print the locations of specified symbol other than function names.
-T: Go through all the tag files listed in GTAGSLIBPATH.
END_OF_HELP
    $curscr->Msg($help);    
}

1;
