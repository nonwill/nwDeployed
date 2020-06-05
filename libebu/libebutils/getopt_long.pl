#                                                         -*- Perl -*-
# Copyright (c) 1997-2006  Motoyuki Kasahara
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

#
# This program is a Perl package running on Perl 4.036 or later.
# The package provides routines to process command line options like
# as GNU getopt_long().
#
# Version:
#     2.0
#
# Interface:
#
#   &getopt_initialize(LIST)
#     Set a list of command line options and initialize internal data
#     for &getopt_long.
#     You must call the routine before calling &getopt_long.
#     Format of each element in the LIST is:
#
#         `CANONICAL-OPTION-NAME [ALIAS-OPTION-NAME...] ARGUMENT-FLAG'
#
#     CANONICAL-OPTION-NAME, ALIAS-OPTION-NAME and ARGUMENT-FLAG fields
#     are separated by spaces or tabs.
#
#     CANONICAL-OPTION-NAME and ALIAS-OPTION-NAME must be either a single
#     character option including preceding `-' (e.g. `-v'), or a long
#     name option including preceding `--' (e.g. `--version').  Whether
#     CANONICAL-OPTION-NAME is single character option or long name
#     option is not significant.
#
#     ARGUMENT-FLAG must be `no-argument', `required-argument' or 
#     `optional-argument'.  If it is set to `required-argument', the
#     option always takes an argument.  If set to `optional-argument',
#     an argument to the option is optional.
#
#     You can put a special element `+' or `-' at the first element in
#     LIST.  See `Details about Option Processing:' for details.
#     If succeeded to initialize, 1 is returned.  Otherwise 0 is
#     returned.
#
#   &getopt_long
#     Get a option name, and if exists, its argument of the leftmost
#     option in @ARGV.
#
#     An option name and its argument are returned as a list with two
#     elements; the first element is CANONICAL-OPTION-NAME of the option,
#     and second is its argument.
#     Upon return, the option and its argument are removed from @ARGV.
#     When you have already got all options in @ARGV, an empty list is
#     returned.  In this case, only non-option elements are left in
#     @ARGV.
#
#     When an error occurs, an error message is output to standard
#     error, and the option name in a returned list is set to `?'.
#
# Example:
#
#     &getopt_intialize('--help -h no-argument', '--version -v no-argument')
#         || die;
#
#     while (($name, $arg) = &getopt_long) {
#         die "For help, type \`$0 --help\'\n" if ($name eq '?');
#         $opts{$name} = $arg;
#     }
#
# Details about Option Processing:
#
#   * There are three processing modes:
#     1. PERMUTE
#        It permutes the contents of ARGV as it scans, so that all the
#        non-option ARGV-elements are at the end.  This mode is default.
#     2. REQUIRE_ORDER
#        It stops option processing when the first non-option is seen.
#        This mode is chosen if the environment variable POSIXLY_CORRECT
#        is defined, or the first element in the option list is `+'.
#     3. RETURN_IN_ORDER
#        It describes each non-option ARGV-element as if it were the
#        argument of an option with an empty name.
#        This mode is chosen if the first element in the option list is
#        `-'.
#
#   * An argument starting with `-' and not exactly `-', is a single
#     character option.
#     If the option takes an argument, it must be specified at just
#     behind the option name (e.g. `-f/tmp/file'), or at the next
#     ARGV-element of the option name (e.g. `-f /tmp/file').
#     If the option doesn't have an argument, other single character
#     options can be followed within an ARGV-element.  For example,
#     `-l -g -d' is identical to `-lgd'.
#     
#   * An argument starting with `--' and not exactly `--', is a long
#     name option.
#     If the option has an argument, it can be specified at behind the
#     option name preceded by `=' (e.g. `--option=argument'), or at the
#     next ARGV-element of the option name (e.g. `--option argument').
#     Long name options can be abbreviated as long as the abbreviation
#     is unique.
#
#   * The special argument `--' forces an end of option processing.
#

{
    package getopt_long;

    $initflag = 0;
    $REQUIRE_ORDER = 0;
    $PERMUTE = 1;
    $RETURN_IN_ORDER = 2;
}


#
# Initialize the internal data.
#
sub getopt_initialize {
    local(@fields);
    local($name, $flag, $canon);
    local($_);

    #
    # Determine odering.
    #
    if ($_[$[] eq '+') {
	$getopt_long'ordering = $getopt_long'REQUIRE_ORDER;
	shift(@_);
    } elsif ($_[$[] eq '-') {
	$getopt_long'ordering = $getopt_long'RETURN_IN_ORDER;
	shift(@_);
    } elsif (defined($ENV{'POSIXLY_CORRECT'})) {
 	$getopt_long'ordering = $getopt_long'REQUIRE_ORDER;
    } else {
	$getopt_long'ordering = $getopt_long'PERMUTE;
    }

    #
    # Parse an option list.
    #
    %getopt_long'optnames = ();
    %getopt_long'argflags = ();

    foreach (@_) {
	@fields = split(/[ \t]+/, $_);
	if (@fields < 2) {
	    warn "$0: (getopt_initialize) too few fields \`$arg\'\n";
	    return 0;
	}
	$flag = pop(@fields);
	if ($flag ne 'no-argument' && $flag ne 'required-argument'
	    && $flag ne 'optional-argument') {
	    warn "$0: (getopt_initialize) invalid argument flag \`$flag\'\n";
	    return 0;
	}

	$canon = '';
	foreach $name (@fields) {
	    if ($name !~ /^-([^-]|-.+)$/) {
		warn "$0: (getopt_initialize) invalid option name \`$name\'\n";
		return 0;
	    } elsif (defined($getopt_long'optnames{$name})) {
		warn "$0: (getopt_initialize) redefined option \`$name\'\n";
		return 0;
	    }
	    $canon = $name if ($canon eq '');
	    $getopt_long'optnames{$name} = $canon;
	    $getopt_long'argflags{$name} = $flag;
	}
    }

    $getopt_long'endflag = 0;
    $getopt_long'shortrest = '';
    @getopt_long'nonopts = ();

    $getopt_long'initflag = 1;
}


#
# When it comes to the end of options, restore PERMUTEd non-option
# arguments to @ARGV.
#
sub getopt_end {
    $getopt_long'endflag = 1;
    unshift(@ARGV, @getopt_long'nonopts);
}


#
# Scan elements of @ARGV for getting an option.
#
sub getopt_long {
    local($name, $arg) = ('', 1);
    local($patt, $key, $ambig, $ch);
    local($_);

    &getopt_initialize(@_) if (!$getopt_long'initflag);
    return () if ($getopt_long'endflag);

    #
    # Take the next argument from @ARGV.
    #
    if ($getopt_long'shortrest ne '') {
	$_ = '-'.$getopt_long'shortrest;
    } elsif (@ARGV == 0) {
	&getopt_end;
	return ();
    } elsif ($getopt_long'ordering == $getopt_long'REQUIRE_ORDER) {
	$_ = shift(@ARGV);
	if (!/^-./) {
	    push(@getopt_long'nonopts, $_);
	    &getopt_end;
	    return ();
	}
    } elsif ($getopt_long'ordering == $getopt_long'PERMUTE) {
	for (;;) {
	    if (@ARGV == 0) {
		&getopt_end;
		return ();
	    }
	    $_ = shift(@ARGV);
	    last if (/^-./);
	    push(@getopt_long'nonopts, $_);
	}
    } else {			# RETURN_IN_ORDER
	$_ = shift(@ARGV);
    }

    #
    # Check for the special option `--'.
    #
    if ($_ eq '--' && $getopt_long'shortrest eq '') {
	#
	# `--' indicates the end of the option list.
	#
	&getopt_end;
	return ();
    }

    #
    # Check for long and short options.
    #
    if (/^(--[^=]+)/ && $getopt_long'shortrest eq '') {
	#
	# Long style option, which start with `--'.
	# Abbreviations for option names are allowed as long as
	# they are unique.
	#
	$patt = $1;
	if (defined($getopt_long'optnames{$patt})) {
	    $name = $patt;
	} else {
	    $ambig = 0;
	    foreach $key (keys(%getopt_long'optnames)) {
		if (index($key, $patt) == 0) {
		    if ($name eq '') {
			$name = $key;
		    } else {
			$ambig = 1;
		    }
		}
	    }
	    if ($ambig) {
		warn "$0: option \`$_\' is ambiguous\n";
		return ('?', '');
	    }
	    if ($name eq '') {
		warn "$0: unrecognized option \`$_\'\n";
		return ('?', '');
	    }
	}

	if ($getopt_long'argflags{$name} eq 'required-argument') {
	    if (/=(.*)$/) {
		$arg = $1;
	    } elsif (0 < @ARGV) {
		$arg = shift(@ARGV);
	    } else {
		warn "$0: option \`$_\' requires an argument\n";
		return ('?', '');
	    }
	} elsif ($getopt_long'argflags{$name} eq 'optional-argument') {
	    if (/=(.*)$/) {
		$arg = $1;
	    } elsif (0 < @ARGV && $ARGV[$[] !~ /^-./) {
		$arg = shift(@ARGV);
	    } else {
		$arg = '';
	    }
	} elsif (/=(.*)$/) {
	    warn "$0: option \`$name\' doesn't allow an argument\n";
	    return ('?', '');
	}
    } elsif (/^(-(.))(.*)/) {
	#
	# Short style option, which start with `-' (not `--').
	#
	($name, $ch, $getopt_long'shortrest) = ($1, $2, $3);

	if (defined($getopt_long'optnames{$name})) {
	    if ($getopt_long'argflags{$name} eq 'required-argument') {
		if ($getopt_long'shortrest ne '') {
		    $arg = $getopt_long'shortrest;
		    $getopt_long'shortrest = '';
		} elsif (0 < @ARGV) {
		    $arg = shift(@ARGV);
		} else {
		    # 1003.2 specifies the format of this message.
		    warn "$0: option requires an argument -- $ch\n";
		    return ('?', '');
		}
	    } elsif ($getopt_long'argflags{$name} eq 'optional-argument') {
		if ($getopt_long'shortrest ne '') {
		    $arg = $getopt_long'shortrest;
		    $getopt_long'shortrest = '';
		} elsif (0 < @ARGV && $ARGV[$[] !~ /^-./) {
		    $arg = shift(@ARGV);
		} else {
		    $arg = '';
		}
	    }
	} elsif (defined($ENV{'POSIXLY_CORRECT'})) {
	    # 1003.2 specifies the format of this message.
	    warn "$0: illegal option -- $ch\n";
	    return ('?', '');
	} else {
	    warn "$0: invalid option -- $ch\n";
	    return ('?', '');
	}
    } else {
	#
	# Only RETURN_IN_ORDER falled into here.
	#
	$arg = $_;
    }

    return ($getopt_long'optnames{$name}, $arg);
}

1;
