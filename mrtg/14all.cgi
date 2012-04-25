#!/usr/bin/perl -w
#
# 14all.cgi
#
# create html pages and graphics with rrdtool for mrtg + rrdtool
#
# (c) 1999,2000 bawidama@users.sourceforge.net
#
# use freely, but: NO WARRANTY - USE AT YOUR OWN RISK!

# if RRDs (rrdtool perl module) is not in the module search path (@INC)
# uncomment the following line and change the path appropriatly:
#use lib '/home/rb1/lib/perl5';

# RCS History - removed as it's available on the web

my $rcsid = '$Id: 14all.cgi,v 1.16.2.13 2000/10/31 11:51:32 rb1 Exp $';
# I used to calculate the release version from the cvs revision.
# but it's getting complicated with branches so I set it manually now
my $version = "14all.cgi 1.0p26";

my $DEBUG = 0;  # set to 1 to resolve problems with the graph generation

use strict;
use CGI;
# don't 'use CGI::Carp qw/fatalsToBrowser/ on WinNT - breaks horribly!
BEGIN {
	eval { require CGI::Carp; import CGI::Carp qw/fatalsToBrowser/ }
		if $^O !~ m/Win/i;
};
use RRDs 1.00008;
use Data::Dumper;
sub print_error($$@);
sub intmax(@);
sub yesorno($);
sub get_graph_params($$$);
sub check_directory($$;$);
sub getrrd($$);
sub getpngdir($$);
sub getpngsize($);
sub errorpng();
sub senderrorpng($$);
sub log_rrdtool_call($$@);

my ($q, $cfgfile, $cfgfiledir);
my ($cgidir, @author, @style);

# set the correct path delimiter
my $SL = '/';
$SL = '\\' if $^O =~ m/Win/;

### where the mrtg.cfg file is
# anywhere in the filespace
#$cfgfile = '/home/mrtg/mrtg.cfg';
# relative to the script
#$cfgfile = 'mrtg.cfg';
# use this so 14all.cgi gets the cfgfile name from the script name 
# (14all.cgi -> 14all.cfg)
$cfgfile = '';

# if you want to store your config files in a different place than your cgis:
$cfgfiledir = '';

### cusotmize the html pages
@author = ( -author => 'bawidama@users.sourceforge.net');
# one possibility to enable stylesheets (second is to use "AddHead[_]:..." in mrtg.cfg)
#@style = ( -style => { -src => 'general.css' });
###

# initialize CGI
$q = new CGI;

# mod_perl changes
# look for the config file
my $meurl = $q->url();
if (defined $q->param('cfg')) {
	$cfgfile = $q->param('cfg');
	$cfgfile = $cfgfiledir.$SL.$cfgfile unless -r $cfgfile;
	my $obj = { author => \@author, q => $q, };
	print_error($obj, "Cannot find the given config file: \<tt>$cfgfile\</tt>")
		unless -r $cfgfile;
} elsif (!$cfgfile) {
	$meurl =~ m{\Q$SL\E([^\Q$SL\E]*)\.(cgi|pl)$};
	$cfgfile = $1 . '.cfg';
	$cfgfile = $cfgfiledir.$SL.$cfgfile unless -r $cfgfile;
}

# read the config file
my ($obj, $err) = new OneForAllConfig($cfgfile, author => \@author, q => $q,
	style => \@style, debug => $DEBUG, SL => $SL,);

unless ($obj) {
	$obj = { q => $q, author => \@author };
	print_error($obj, "cannot load config file \<tt>$cfgfile\</tt>: $err");
}

my @headeropts = ( @author, @style, -bgcolor => $obj->get('config','background'));
my $icondir = $obj->get('config','icondir');

# the footer we print on every page
my $footer = <<"EOT" . $q->end_html;
<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0>
  <TR>
    <TD WIDTH=63><A ALT="MRTG"
    HREF="http://ee-staff.ethz.ch/~oetiker/webtools/mrtg/mrtg.html"><IMG
    BORDER=0 SRC="${icondir}mrtg-l.gif"></A></TD>
    <TD WIDTH=25><A ALT=""
    HREF="http://ee-staff.ethz.ch/~oetiker/webtools/mrtg/mrtg.html"><IMG
    BORDER=0 SRC="${icondir}mrtg-m.gif"></A></TD>
    <TD WIDTH=388><A ALT=""
    HREF="http://ee-staff.ethz.ch/~oetiker/webtools/mrtg/mrtg.html"><IMG
    BORDER=0 SRC="${icondir}mrtg-r.gif"></A></TD>
  </TR>
</TABLE>
<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0>
  <TR VALIGN=top>
  <TD WIDTH=88 ALIGN=RIGHT><FONT FACE="Arial,Helvetica" SIZE=2>Version 2.8.x-rrd</FONT></TD>
  <TD WIDTH=388 ALIGN=RIGHT><FONT FACE="Arial,Helvetica" SIZE=2>
  <A HREF="http://ee-staff.ethz.ch/~oetiker/">Tobias Oetiker</A>
  <A HREF="mailto:oetiker\@ee.ethz.ch">&lt;oetiker\@ee.ethz.ch&gt;</A> and
  <A HREF="http://www.bungi.com">Dave&nbsp;Rand</A>&nbsp;
  <A HREF="mailto:dlr\@bungi.com">&lt;dlr\@bungi.com&gt;</A>
  <TR VALIGN=top>
  <TD WIDTH=88 ALIGN=RIGHT><FONT FACE="Arial,Helvetica" SIZE=2>$version</FONT></TD>
  <TD WIDTH=388 ALIGN=RIGHT><FONT FACE="Arial,Helvetica" SIZE=2>
  <A HREF="http://www.wh-hms.uni-ulm.de/~widi/">Rainer&nbsp;Bawidamann</A>&nbsp;
  <A HREF="mailto:bawidama\@users.sourceforge.net">&lt;bawidama\@users.sourceforge.net&gt;</A></FONT>
  </TD>
</TR>
</TABLE>
EOT

### the main switch
# the modes:
# if parameter "dir" is given show a list of the targets in this "directory"
# elsif parameter "png" is given show a graphic for the target given w/ parameter "log"
# elsif parameter "log" is given show the page for this target
# else show a list of directories and of targets w/o directory
# parameter "cfg" can hold the name of the config file to use
if (defined $q->param('dir')) {
	# show a list of targets in the given directory
	my $dir = $q->param('dir');
	my @httphead;
	if (yesorno($obj->get('config','{writeexpires'))) {
		push @httphead, (-expires => '+' . int($obj->get('config','interval')) . 'm');
	}
	if (yesorno($obj->get('config','refresh'))) {
		push @httphead, (-refresh => $obj->get('config','refresh'));
	}
	my @htmlhead = (-title => "MRTG/RRD - Group $dir", @headeropts);
	if ($obj->get('target','_','addhead')) {
		push @htmlhead, (-head => $obj->get('target','_','addhead'));
	}
	print $q->header(@httphead), $q->start_html(@htmlhead);
	print $q->h1("Available Targets"),"\n\<table width=100\%>\n";
	my $cfgstr = (defined $q->param('cfg') ? "&cfg=".$q->param('cfg') : '');
	my $column = 0;
	foreach my $tar (@{$obj->{sorted}}) {
		my $small = 0;
		if (yesorno($obj->get('option',$tar,'indexgraph'))) {
			$small = $obj->get('option',$tar,'indexgraph');
		}
		next if $tar =~ m/^[\$\^\_]$/; # _ is not a real target
		next if $obj->get('target',$tar,'directory') ne $dir;
		print '<tr>' if $column == 0;
		print '<td>',
			$q->p($q->a({href => "$meurl?log=$tar$cfgstr"}, $obj->get('target',$tar,'title')));
		print $q->a({href => "$meurl?log=$tar$cfgstr"},
			$q->img({src => "$meurl?log=$tar&png=$small&small=1$cfgstr",
				alt => "index-graph",
				getpngsize(getpngdir($obj,$tar)."$tar-$small.png")})
			) if $small;
		print "\</td>\n";
		$column++;
		if ($column >= $obj->get('config','columns')) {
			$column = 0;
			print '</tr>';
		}
	}
	if ($column != 0 and $column < $obj->get('config','columns')) {
		print '<td>&nbsp;</td>' x ($obj->get('config','columns') - $column),"\</tr>\n";
	}
	print '</table>', $footer;
} elsif (defined $q->param('png')) {
	# send a graphic, create it if necessary
	print_error($obj,"CGI call error") if (!defined $q->param('log'));
	my $png = $q->param('png');
	my $log = $q->param('log');
	$obj->{'log'} = $log;
	my (@args, $save_args);
	if (exists $obj->{confhash}) {
		my $rrdcall = $obj->{confhash}{"rrdcall $log $png"};
		if ($rrdcall) {
			@args = split(' ',$rrdcall);
			map { $_ =~ s/&(..)/chr($1)/ge } @args;
			$obj->{maxage} = $obj->{confhash}{"maxage $log $png"} || 300;
		}
	}
	unless (@args) {
		get_rrd_call($obj,$log,$png,\@args);
		my @saveargs = @args;
		map { $_ =~ s/([ &])/'&'.ord($1)/ge } @saveargs;
		my $value = join(' ',@saveargs);
		# set will make cache writeable
		$obj->set("rrdcall $log $png", $value);
		$obj->set("maxage $log $png", $obj->{maxage});
	}
	# the first element of @args if the name of the graph file

	# fire up rrdtool
	my ($a, $rrdx, $rrdy) = RRDs::graph(@args);
	my $e = RRDs::error();
	log_rrdtool_call($obj,$e,'graph',@args);
	# if the file given to rrdtool is not a png file rrdtool won't report
	# an error (fixed in 1.0.11). we have to check the reported image size:
	if ($e or $rrdx == 0 and $rrdy == 0) {
		my ($pngdir) = ($args[0] =~ m|^(.*?)(\Q$SL\E.+?)?$|);
		senderrorpng($obj, "cannot write to graph dir $pngdir"
			.($e ? "\nrrdtool error: $e" : '')) if ! -w $pngdir;
		senderrorpng($obj, "cannot write $args[0]"
				.($e ? "\nrrdtool error: $e" : "\nadditional info: file is not a PNG file"))
			if (-e $args[0] and !-w _);
		unlink $args[0] or senderrorpng($obj,"cannot delete file $args[0]")
			if -e $args[0]; # do we get here if it does not exist?
		($a, $rrdx, $rrdy) = RRDs::graph(@args);
		$e = RRDs::error();
		log_rrdtool_call($obj,$e,'graph',@args);
	}
	if ($e) {
		senderrorpng($obj, "RRDTool error: $e");
	} elsif ($rrdx == 0 and $rrdy == 0) {
		senderrorpng($obj, "Cannot create graph or wrong format")
			or print_error($obj,"Cannot create graph or wrong format");
	}
	# no error, try to send the file
	open(PNG, "<$args[0]")
		or senderrorpng($obj, "cannot read graph file")
		or print_error($obj,"cannot read graph file");
	my @httphead = (-type => "image/png");
	if (yesorno($obj->get('config','writeexpires'))) {
		push @httphead, (-expires => '+'.$obj->{'maxage'}.'s');
	}
	print $q->header(@httphead);
	binmode(PNG); binmode(STDOUT);
	while(read PNG, my $buf, 16384) { print STDOUT $buf; }
	close PNG;
} elsif (defined $q->param('log')) {
	# show the graphics for one target
	my $log = $q->param('log');
	print_error($obj,"Target $log unknown") unless ($obj->get('target',$log,'target'));
	my $title;
	# user defined title?
	if ($obj->get('target',$log,'title')) {
		$title = $obj->get('target',$log,'title');
	} else {
		$title = "MRTG/RRD - Target $log";
	}
	my @httphead;
	if (yesorno($obj->get('config','writeexpires'))) {
		push @httphead, (-expires => '+' . int($obj->get('config','interval')) . 'm');
	}
	if (yesorno($obj->get('config','refresh'))) {
		push @httphead, (-refresh => $obj->get('config','refresh'));
	}
	my @htmlhead = (-title => $title, @headeropts);
	if ($obj->get('target',$log,'addhead')) {
		push @htmlhead, (-head => $obj->get('target',$log,'addhead'));
	}
	print $q->header(@httphead), $q->start_html(@htmlhead);
	# user defined header line? (should exist as mrtg requires it)
	if (defined $obj->get('target',$log,'pagetop')) {
		print $obj->get('target',$log,'pagetop'),"\n";
	} else {
		print $q->h1("Target ".$obj->get('target',$log,'title')),"\n";
	}
	my $lasttime = RRDs::last(getrrd($obj,$log));
	log_rrdtool_call($obj,'','last',getrrd($obj,$log));
	print $q->hr,
		"The statistics were last updated: ",$q->b(scalar(localtime($lasttime))),
		$q->hr if $lasttime;
	my $sup = $obj->get('target',$log,'suppress');
	my $url = "$meurl?log=$log";
	my $tmpcfg = $q->param('cfg');
	$url .= "&cfg=$tmpcfg" if defined $tmpcfg;
	$url .= "&png";
	# the header lines and tags for the graphics
	if ($sup !~ /d/) {
		print $q->h2("'Daily' graph (5 Minute Average)"),"\n",
			$q->img({src => "$url=daily", alt => "daily-graph",
				getpngsize(getpngdir($obj,$log)."$log-daily.png")}
			), "\n";
	}
	if ($sup !~ /w/) {
		print $q->h2("'Weekly' graph (30 Minute Average)"),"\n",
			$q->img({src => "$url=weekly", alt => "weekly-graph",
				getpngsize(getpngdir($obj,$log)."$log-weekly.png")}
			), "\n";
	}
	if ($sup !~ /m/) {
		print $q->h2("'Monthly' graph (2 Hour Average)"),"\n",
			$q->img({src => "$url=monthly", alt => "monthly-graph",
				getpngsize(getpngdir($obj,$log)."$log-monthly.png")}
			), "\n";
	}
	if ($sup !~ /y/) {
		print $q->h2("'Yearly' graph (1 Day Average)"),"\n",
			$q->img({src => "$url=yearly", alt => "yearly-graph",
				getpngsize(getpngdir($obj,$log)."$log-yearly.png")}
			), "\n";
	}
	if ($obj->get('target',$log,'pagefoot')) {
		print $obj->get('target',$log,'pagefoot');
	}
	print $footer;
} else {
	# no parameter - show a list of directories and targets without "Directory[...]" (aka root-targets)
	my @httphead;
	if (yesorno($obj->get('config','writeexpires'))) {
		push @httphead, (-expires => '+1d'); # how often do you add targets?
	}
	if (yesorno($obj->get('config','refresh'))) {
		push @httphead, (-refresh => $obj->get('config','refresh'));
	}
	my @htmlhead = (-title => "MRTG/RRD $version", @headeropts);
	if ($obj->get('target','_','addhead')) {
		push @htmlhead, (-head => $obj->get('target','_','addhead'));
	}
	print $q->header(@httphead), $q->start_html(@htmlhead);
	my (@dirs, %dirs, @logs);
	# get the list of directories and "root"-targets
	foreach my $tar (@{$obj->{sorted}}) {
		next if $tar =~ m/^[_\$\^]$/; # pseudo targets
		my $dir = $obj->get('target',$tar,'directory');
		if ($dir) {
			next if exists $dirs{$dir};
			$dirs{$dir} = $tar;
			push @dirs, $dir;
		} else {
			push @logs, $tar;
		}
	}
	my $cfgstr = (defined $q->param('cfg') ? "&cfg=".$q->param('cfg') : '');
	print $q->h1("Available Targets"),"\n";
	my $confcolumns = $obj->get('config','columns');
	if ($#dirs > -1) {
		print $q->h2("Directories"),"\n\<table width=100\%>\n";
		my $column = 0;
		foreach my $tar (@dirs) {
			print '<tr>' if $column == 0;
			(my $link = $tar) =~ s/ /\+/g;
			print $q->td($q->a({href => "$meurl?dir=$link$cfgstr"},
				$tar)),"\n";
			$column++;
			if ($column >= $confcolumns) {
				$column = 0;
				print '</tr>';
			}
		}
		if ($column != 0 and $column < $confcolumns) {
			print '<td>&nbsp;</td>' x ($confcolumns - $column),"\</tr>\n";
		}
		print '</table><hr>';
	}
	if ($#logs > -1) {
		print $q->h2("Targets"),"\n\<table width=100\%>\n";
		my $column = 0;
		foreach my $tar (@logs) {
			my $small = 0;
			if (yesorno($obj->get('option',$tar,'indexgraph'))) {
				$small = $obj->get('option',$tar,'indexgraph');
			}
			next if $tar =~ m/^[\$\^_]$/;
			next if $obj->get('target',$tar,'directory');
			print '<tr>' if $column == 0;
			print '<td>',
				$q->p($q->a({href => "$meurl?log=$tar$cfgstr"},
					$obj->get('target',$tar,'title')));
			print $q->a({href => "$meurl?log=$tar$cfgstr"},
				$q->img({src => "$meurl?log=$tar&png=$small&small=1$cfgstr",
					alt => "index-graph",
					getpngsize(getpngdir($obj,$tar)."$tar-$small.png")}))
				if $small;
			print "\</td>\n";
			$column++;
			if ($column >= $confcolumns) {
				$column = 0;
				print '</tr>';
			}
		}
		if ($column != 0 and $column < $confcolumns) {
			print '<td>&nbsp;</td>' x ($confcolumns - $column),"\</tr>\n";
		}
		print '</table>';
	}
	print $footer;
}
exit 0;

sub get_rrd_call ($$$$) {
	my ($obj, $log, $png, $argsref) = @_;
	my ($start, $end, $maxage, $xs, $ys) = get_graph_params($obj, $log, $png);
	print_error($obj,"undefined graph") unless $start;
	my $q = $obj->{q};
	# save maxage for refresh/expire setting
	$obj->{'maxage'} = $maxage;

	my $rrd = getrrd($obj, $log);
	# escape ':' and '\' with \ in $rrd
	# (rrdtool replaces '\:' by ':' and '\\' by '\')
	$rrd =~ s/([:\\])/\\$1/g;
	my $pngdir = getpngdir($obj,$log);
	my $pngfile = ${pngdir}."${log}-${png}.png";
	
	# build the rrd command line: set the starttime and the graphics format (PNG)
	@$argsref = ($pngfile, '-s', $start, '-e', $end, '-a', 'PNG');
	# if it's not a small picture set the legends
	my ($l1,$l2,$l3,$l4,$li,$lo) = ('','','','','','');
	my ($ri, $ro) = ('','');

	push @$argsref, '-w', $xs, '-h', $ys;
	if (!defined $q->param('small')) {
		if ($obj->get('target',$log,'ylegend')) {
			push @$argsref, '-v', $obj->get('target',$log,'ylegend'); }
		if ($obj->get('target',$log,'legend1')) {
			$l1 = ":".$obj->get('target',$log,'legend1')."\\l"; }
		if ($obj->get('target',$log,'legend2')) {
			$l2 = ":".$obj->get('target',$log,'legend2')."\\l"; }
		if ($obj->get('target',$log,'legend3') ne '--CALC--') {
			$l3 = ":".$obj->get('target',$log,'legend3')."\\l";
		} else {
			$l3 = ":Maximal 5 Minute ".$obj->get('target',$log,'legend1')."\\l";
		}
		if ($obj->get('target',$log,'legend4') ne '--CALC--') {
			$l4 = ":".$obj->get('target',$log,'legend4')."\\l";
		} else {
			$l4 = ":Maximal 5 Minute ".$obj->get('target',$log,'legend2')."\\l";
		}
		if ($obj->get('target',$log,'legendi')) {
			$li = $obj->get('target',$log,'legendi'); }
		else {	$li = "In: "; }
		$li =~ s':'\\:'; # ' quote :
		if ($obj->get('target',$log,'legendo')) {
			$lo = $obj->get('target',$log,'legendo'); }
		else {	$lo = "Out:"; }
		$lo =~ s':'\\:'; # ' quote :
		if ($obj->get('option',$log,'integer')) {
			$li .= ' %9.0lf';
			$lo .= ' %9.0lf';
			$ri = '%3.0lf%%';
			$ro = '%3.0lf%%';
		} else {
			$li .= ' %8.3lf';
			$lo .= ' %8.3lf';
			$ri = '%6.2lf%%';
			$ro = '%6.2lf%%';
		}
		if ($obj->get('target',$log,'kmg')) {
			$li .= ' %s';
			$lo .= ' %s';
			if ($obj->get('target',$log,'kilo')) {
				push @$argsref, '-b', $obj->get('target',$log,'kilo');
			}
			if ($obj->get('target',$log,'shortlegend')) {
				$li .= $obj->get('target',$log,'shortlegend');
				$lo .= $obj->get('target',$log,'shortlegend');
			}
		}
	}
	my $factor = 1; # should we scale the values?
	if ($obj->get('option',$log,'perminute')) {
		$factor = 60; # perminute -> 60x
	} elsif ($obj->get('option',$log,'perhour')) {
		$factor = 3600; # perhour -> 3600x
	}
	if ($obj->get('option',$log,'bits')) {
		$factor *= 8; # bits instead of bytes -> 8x
	}
	# let the user give an arbitrary factor:
	if ($obj->get('option',$log,'factor') and
		$obj->get('option',$log,'factor') =~ m/^[-+]?\d+(.\d+)?([eE][+-]?\d+)?$/)
	{
		$factor *= 0+$obj->get('option',$log,'factor');
	}
	my $pngchar = substr($png,0,1);
	if ($pngchar and $obj->get('target',$log,'unscaled') and
		   $obj->get('target',$log,'unscaled') =~ m/$pngchar/) {
		my $max = intmax($obj->get('target',$log,'maxbytes'),
			$obj->get('target',$log,'maxbytes1'),
			$obj->get('target',$log,'maxbytes2'),
			$obj->get('target',$log,'absmax'));
		$max *= $factor;
		push @$argsref, '-l', 0, '-u', $max, '-r';
	} elsif (yesorno($obj->get('option',$log,'logarithmic'))) {
		push @$argsref, '-o';
	}
	push @$argsref,'--alt-y-grid','--lazy','-c','MGRID#ee0000','-c','GRID#000000';
	# now build the graph calculation commands
	# ds0/ds1 hold the normal data sources to graph/gprint
	my ($ds0, $ds1) = ('in', 'out');
	push @$argsref, "DEF:$ds0=$rrd:ds0:AVERAGE", "DEF:$ds1=$rrd:ds1:AVERAGE";
	if (defined $obj->get('option',$log,'unknaszero')) {
		push @$argsref, "CDEF:uin=$ds0,UN,0,$ds0,IF",
			"CDEF:uout=$ds1,UN,0,$ds1,IF";
		($ds0, $ds1) = ('uin', 'uout');
	}
	if ($factor != 1) {
		# scale the values. we need a CDEF for this
		push @$argsref, "CDEF:fin=$ds0,$factor,*","CDEF:fout=$ds1,$factor,*";
		($ds0, $ds1) = ('fin', 'fout');
	}
	my $maximum0 = $obj->get('target',$log,'maxbytes1') || $obj->get('target',$log,'maxbytes');
	my $maximum1 = $obj->get('target',$log,'maxbytes2') || $obj->get('target',$log,'maxbytes');
	$maximum0 = 1 unless $maximum0;
	$maximum1 = 1 unless $maximum1;
	# ps0/ps1 hold the percentage data source for gprint
	my ($ps0, $ps1) = ('pin', 'pout');
	push @$argsref, "CDEF:pin=$ds0,$maximum0,/,100,*,$factor,/",
		"CDEF:pout=$ds1,$maximum1,/,100,*,$factor,/";

	# now for the peak graphs / maximum values
	# mx0/mx1 hold the maximum data source for graph/gprint
	my ($mx0, $mx1) = ($ds0, $ds1);
	# px0/px1 hold the maximum pecentage data source for gprint
	my ($px0, $px1) = ($ps0, $ps1);
	if (!defined $q->param('small')) {
		# the defs for the maximum values: for the legend ('MAX') and probabely
		# for the 'withpeak' graphs
		push @$argsref, "DEF:min=$rrd:ds0:MAX", "DEF:mout=$rrd:ds1:MAX";
		($mx0, $mx1) = ('min', 'mout');
		if (defined $obj->get('option',$log,'unknaszero')) {
			push @$argsref, "CDEF:umin=$mx0,UN,0,$mx0,IF",
				"CDEF:umout=$mx1,UN,0,$mx1,IF";
			($mx0, $mx1) = ('umin', 'umout');
		}
		if ($factor != 1) {
			# scale the values. we need a CDEF for this
			push @$argsref, "CDEF:fmin=$mx0,$factor,*","CDEF:fmout=$mx1,$factor,*";
			($mx0, $mx1) = ('fmin', 'fmout');
		}
		# draw peak lines if configured
		my $wp = $obj->get('target',$log,'withpeak');
		if (substr($png,0,1) =~ /[$wp ]/) {
			push @$argsref, "AREA:$mx0#006600$l3", "LINE1:$mx1#ff00ff$l4";
			push @$argsref, "CDEF:pmin=$mx0,$maximum0,/,100,*,$factor,/",
				"CDEF:pmout=$mx1,$maximum1,/,100,*,$factor,/";
			($px0, $px1) = ('pmin', 'pmout');
			if (yesorno($obj->get('option',$log,'graphtotal'))) {
				push @$argsref, "CDEF:mtotal=$mx0,$mx1,+", "LINE1:mtotal#ff5050:Total MAX\\l";
			}
		}
	}
	# the commands to draw the values
	push @$argsref, "AREA:$ds0#00cc00$l1", "LINE1:$ds1#0000ff$l2";
	if (yesorno($obj->get('option',$log,'graphtotal'))) {
		push @$argsref, "CDEF:total=$ds0,$ds1,+", "LINE1:total#ffa050:Total AVG\\l";
	}
	if (!defined $q->param('small')) {
		# print the legends
		if ($obj->get('option',$log,'nopercent')) {
			push @$argsref,
				"GPRINT:$mx0:MAX:Maximal $li",
				"GPRINT:$mx1:MAX:Maximal $lo\\l",
				"GPRINT:$ds0:AVERAGE:Average $li",
				"GPRINT:$ds1:AVERAGE:Average $lo\\l",
				"GPRINT:$ds0:LAST:Current $li",
				"GPRINT:$ds1:LAST:Current $lo\\l";
		} else {
			push @$argsref,
				"GPRINT:$mx0:MAX:Maximal $li",
				"GPRINT:$px0:MAX:($ri)",
				"GPRINT:$mx1:MAX:Maximal $lo",
				"GPRINT:$px1:MAX:($ro)\\l",
				"GPRINT:$ds0:AVERAGE:Average $li",
				"GPRINT:$ps0:AVERAGE:($ri)",
				"GPRINT:$ds1:AVERAGE:Average $lo",
				"GPRINT:$ps1:AVERAGE:($ro)\\l",
				"GPRINT:$ds0:LAST:Current $li",
				"GPRINT:$ps0:LAST:($ri)",
				"GPRINT:$ds1:LAST:Current $lo",
				"GPRINT:$ps1:LAST:($ro)";
		}
	}
	return;
}

sub print_error($$@)
{
	my $obj = shift;
	my $q = $obj->{q};
	print $q->header(),
		$q->start_html(
			-title => 'MRTG/RRD index.cgi - Skript error',
			@{$obj->{author}},
			-bgcolor => '#ffffff'
		),
		$q->h1('Skript Error'),
		@_, $q->end_html();
	exit 0;
}

sub intmax(@)
{
	my (@p) = @_;
	my $max = 0;
	foreach my $n (@p) {
		$max = int($n) if defined $n and int($n) > $max;
	}
	return $max;
}

sub yesorno($)
{
	my $opt = shift;
	return 0 unless defined $opt;
	return 0 if $opt =~ /^((no?)|(false)|0)$/i;
	return 1;
}

sub get_graph_params($$$)
{
	my ($obj, $tar, $graph) = @_;
	my ($start, $end, $maxage, $xs, $ys);
	my $v;
	if ($v = $obj->get('option',$tar,"graph $graph")) {
		($start, $end, $maxage) = split / +/, $v;
	} elsif ($v = $obj->get('option','_',"graph $graph")) {
		($start, $end, $maxage) = split / +/, $v;
	} else {
		return ();
	}
	if ($v = $obj->get('option',$tar,"graphsize $graph")) {
		($xs, $ys) = split / +/, $v;
	} elsif ($v = $obj->get('option',$tar,"graphsize")) {
		($xs, $ys) = split / +/, $v;
	} elsif ($v = $obj->get('option','_',"graphsize $graph")) {
		($xs, $ys) = split / +/, $v;
	} elsif ($v = $obj->get('option','_',"graphsize")) {
		($xs, $ys) = split / +/, $v;
	} else {
		$xs = $obj->get('target',$tar,'xsize');
		$ys = $obj->get('target',$tar,'ysize');
	}
	return ($start, $end, $maxage, $xs, $ys);
}

sub getrrd($$)
{
	my ($obj, $tar) = @_;
	my $rrd = $obj->get('config','logdir');
	$rrd .= $obj->{SL} unless $rrd =~ m|\Q$obj->{SL}\E$|;
	$rrd .= $obj->get('target',$tar,'directory');
	$rrd .= $obj->{SL} unless $rrd =~ m|\Q$obj->{SL}\E$|;
	$rrd .= $tar . '.rrd';
	return $rrd;
}

sub getpngdir($$)
{
	my ($obj, $tar) = @_;
	# a small cache:
	if (exists $obj->{"__pngdir__$tar"}) {
		return $obj->{"__pngdir__$tar"};
	}
	my $pngdir = $obj->get('config','imagedir');
	$pngdir .= $obj->{SL} unless $pngdir =~ m|\Q$obj->{SL}\E$|;
	$pngdir .= $obj->get('target',$tar,'directory');
	$pngdir .= $obj->{SL} unless $pngdir =~ m|\Q$obj->{SL}\E$|;
	if (!-w $pngdir) {
		if ($^O =~ m/Win/i) {
			$pngdir = $ENV{'TEMP'};
			$pngdir = $ENV{'TMP'} unless $pngdir;
			$pngdir = '' unless $pngdir;
			$pngdir .= $obj->{SL} unless $pngdir =~ m|\Q$obj->{SL}\E$|;
		} else { 
			$pngdir = '/tmp/';
		}
	}
	$obj->{"__pngdir__$tar"} = $pngdir;
	return $pngdir;
}

use IO::File;

sub pngstring() { return chr(137)."PNG".chr(13).chr(10).chr(26).chr(10); };

sub getpngsize($)
{
	my ($file) = @_;
	my $fh = new IO::File $file;
	return () unless defined $fh;
	my $line;
	if (sysread($fh, $line, 8) != 8 or $line ne pngstring()) {
		$fh->close;
		return ();
	}
	CHUNKS: while(1) {
		last CHUNKS if (sysread($fh, $line, 8) != 8);
		my ($chunksize, $type) = unpack "Na4", $line;
		if ($type ne "IHDR") {
			last CHUNKS if (sysread($fh, $line, $chunksize + 4) != $chunksize + 4);
			next CHUNKS;
		}
		last CHUNKS if (sysread($fh, $line, 8) != 8);
		$fh->close;
		my ($x, $y) = unpack("NN", $line);
		return ('-width' => "$x", '-height' => "$y");
	}
	$fh->close;
	return ();
}

# this data contains a small png with the text:
# "error: cannot create graph"

sub errorpng()
{
	return (
		137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,187,0,0,0,29,4,3,0,
		0,0,0,251,0,170,0,0,0,4,103,65,77,65,0,0,177,143,11,252,97,5,0,0,0,30,80,
		76,84,69,255,0,0,255,93,93,255,128,128,255,155,155,255,176,176,255,195,
		195,255,212,212,255,227,227,255,241,241,255,255,255,17,191,146,253,0,0,
		0,56,116,69,88,116,83,111,102,116,119,97,114,101,0,88,86,32,86,101,114,
		115,105,111,110,32,51,46,49,48,97,32,32,82,101,118,58,32,49,50,47,50,57,
		47,57,52,32,40,80,78,71,32,112,97,116,99,104,32,49,46,50,41,221,21,46,73,
		0,0,2,40,73,68,65,84,120,218,237,147,177,107,219,64,20,198,159,34,233,92,
		109,130,180,132,108,55,180,78,189,169,113,8,220,166,80,106,208,230,102,
		48,237,118,56,246,153,219,28,7,2,183,165,77,23,109,142,101,157,244,254,
		219,62,73,198,113,106,211,150,144,108,254,166,143,167,79,63,221,125,167,
		3,216,107,175,189,94,65,136,56,223,26,58,166,224,207,71,6,114,3,175,148,
		220,10,136,66,47,183,134,44,254,115,114,48,252,55,126,87,192,92,248,24,
		254,237,173,70,110,246,95,120,182,184,17,231,242,4,103,160,79,34,213,12,
		117,148,224,204,205,211,159,96,205,18,24,254,162,26,105,189,198,66,29,149,
		96,80,10,172,29,64,154,91,55,83,30,22,92,124,43,33,152,60,86,75,229,196,
		172,204,68,42,83,85,114,157,70,4,97,182,218,121,121,150,187,56,41,0,111,
		75,254,253,54,141,197,13,64,203,154,184,138,78,114,223,158,47,222,229,156,
		28,128,95,104,235,150,182,115,159,244,133,53,211,160,208,215,27,71,43,153,
		253,36,238,223,23,96,250,250,42,84,43,188,55,58,180,174,117,16,202,208,
		156,166,97,114,87,237,185,211,239,72,138,230,78,217,30,126,200,220,204,
		35,199,225,227,67,139,240,167,189,11,33,197,244,120,17,60,180,178,39,229,
		60,128,136,217,156,74,211,117,227,53,30,18,164,29,211,115,11,186,155,1,
		155,87,120,93,157,123,204,104,77,145,65,194,215,14,68,228,90,119,9,30,173,
		148,108,22,72,246,20,63,167,4,91,64,71,234,213,48,116,204,89,193,31,241,
		57,28,173,240,46,167,104,214,237,30,229,135,21,158,92,8,162,239,85,209,
		100,22,72,17,251,187,241,94,93,14,64,151,6,250,171,143,108,225,173,241,
		235,114,146,233,177,172,162,142,13,174,91,85,57,228,160,41,39,3,205,147,
		166,156,77,60,98,86,227,65,163,13,117,243,189,35,196,31,30,150,107,124,
		36,176,228,111,170,67,71,12,235,232,172,133,101,118,128,146,28,253,160,
		77,52,193,84,138,20,227,77,188,82,106,232,73,104,115,120,171,46,160,71,
		3,26,58,131,49,135,193,229,136,238,141,130,17,244,184,171,46,193,165,39,
		78,239,170,142,142,67,71,125,30,194,32,38,71,249,193,200,82,212,31,183,
		99,241,101,76,247,207,219,125,223,158,41,49,111,126,134,202,70,47,9,110,
		228,151,230,238,21,241,123,237,245,162,250,13,181,158,203,16,233,3,210,
		153,0,0,0,7,116,73,77,69,7,208,1,19,13,28,15,223,54,180,209,0,0,0,0,73,
		69,78,68,174,66,96,130
	);
}

sub senderrorpng($$) {
	my ($obj, $text) = @_;
	binmode STDOUT;
	if (defined $obj->get('option',$obj->{log},'errorpic')) {
		open(PNG, $obj->get('option',$obj->{log},'errorpic')) || return 0;
		print $obj->{q}->header(-type => "image/png", -expires => 'now');
		binmode(PNG);
		while(read PNG, my $buf, 16384) { print STDOUT $buf; }
		close PNG;
		exit 0;
	}
	if ($obj->{debug}) {
		my @textsplit = split(/\n/, $text);
		my $len = 0;
		my $max = sub { $_[0] > $_[1] ? $_[0] : $_[1] };
		my @rrdargs;
		foreach (@textsplit) {
			$len = &$max($len, length($_));
			push @rrdargs, "COMMENT:$_\\l";
		}
		eval { require GD; 1; };
		unless ($@||1) {
			my $ys = @textsplit * (GD::gdMediumBoldFont()->height + 5);
			my $xs = $len * GD::gdMediumBoldFont()->width();
			my $starty = 10;
			my $im = new GD::Image($xs + 20, $ys + 2 * $starty);
			my $back = $im->colorAllocate(255,255,255);
			$im->transparent($back);
			my $red = $im->colorAllocate(255,0,0);
			$im->filledRectangle(0,0,$xs-1,$ys-1,$back);
			foreach $text (@textsplit) {
				$im->string(GD::gdMediumBoldFont(), 10, $starty, $text, $red);
				$starty += 5 + GD::gdMediumBoldFont()->height;
			}
			if ($GD::VERSION lt '1.20') {
				print $obj->{q}->header(-type => "image/gif", -expires => 'now');
				eval 'print $im->gif';
			} elsif ($GD::VERSION ge '1.20') {
				print $obj->{q}->header(-type => "image/png", -expires => 'now');
				eval 'print $im->png';
			}
			exit 0;
		}
		if (!$ENV{MOD_PERL}) {
			# create a graphic with rrdtool
			$len = &$max($len*6-60,50);
			unshift @rrdargs, ('-', '-w', $len, '-h', 10, '-c', 'FONT#ff0000');
			my $pid = open(P, "-|");
			if (defined $pid && $pid == 0) {
				RRDs::graph(@rrdargs);
				exit 0;
			} elsif ($pid) {
				local $/ = undef;
				my $png = <P>;
				close P;
				if (defined $png) {
					print $obj->{q}->header(-type => "image/png", -expires => 'now'),
						$png;
					exit 0;
				}
			}
		}
	}
	print $obj->{q}->header(-type => "image/png", -expires => 'now');
	print pack("C*", errorpng());
	exit 0;
}

sub log_rrdtool_call($$@) {
	my $obj = shift;
	my $error = shift;
	return unless yesorno($obj->get('config','rrdtoollog'));
	unless (open(LOG, '>>'.$obj->get('config','rrdtoollog'))) {
		print STDERR "cannot log rrdtool call: $!\n";
		return;
	}
	print LOG "\n# call to rrdtool:\nrrdtool @_\n";
	if ($error) {
		print LOG "# gave error: $error\n";
	} else {
		print LOG "# completed without error\n";
	}
	close LOG;
}

#### a package for the config

package OneForAllConfig;

BEGIN { @AnyDBM_File::ISA = qw/DB_File GDBM_File NDBM_File SDBM_File ODBM_File/; };
use Fcntl; # POSIX?
use AnyDBM_File;

sub new {
	my $this = shift;
	my $class = ref($this) || $this;
	my $cfgfile = shift;
	my $self = { @_ };
	bless $self, $class;
	my $cachefile = $cfgfile . '.cache';
	my %confhash;
	my $cfgmtime = (stat($cfgfile))[9];
	if (tie(%confhash, 'AnyDBM_File', $cachefile, O_RDONLY, 0640)) {
		if (exists $confhash{'14all-cache'} 
				&& $confhash{'14all-cache'} eq 'v1'
				&& exists $confhash{'configfilemtime'}
				&& $confhash{'configfilemtime'} == $cfgmtime)
		{
			$self->{'confhash'} = \%confhash;
			$self->{'sorted'} = [split /,/, $confhash{sorted}];
			$self->{'cachefile'} = $cachefile;
			#print STDERR "using config cache\n";
		} else {
			untie %confhash;
		}
	}
	unless (exists $self->{confhash}) {
		#print STDERR "reading config file\n";
		my $err = $self->read_mrtg_config($cfgfile);
		$err ||= $self->fixconfig();
		if ($err) {
			return (wantarray ? (undef, $err) : undef);
		}
		unless (tie(%confhash, 'AnyDBM_File', $cachefile, O_RDWR|O_CREAT, 0600)) {
			return $self;
		}
		#print STDERR "writing config cache\n";
		%confhash = ();
		foreach my $one (keys %{$self->{target}}) {
			next if $one eq '^' || $one eq '$';
			foreach my $two (keys %{$self->{target}{$one}}) {
				$confhash{"target $one $two"} = $self->{target}{$one}{$two};
			}
		}
		foreach my $one (keys %{$self->{option}}) {
			next if $one eq '^' || $one eq '$';
			foreach my $two (keys %{$self->{option}{$one}}) {
				$confhash{"option $one $two"} = $self->{option}{$one}{$two};
			}
		}
		foreach my $one (keys %{$self->{config}}) {
			$confhash{"config $one"} = $self->{config}{$one};
		}
		$confhash{'sorted'} = join(',',@{$self->{sorted}});
		$confhash{'14all-cache'} = 'v1';
		$confhash{'configfilemtime'} = $cfgmtime;
		untie %confhash;
		$self->{'cachefile'} = $cachefile;
	}
	return $self;
}

sub get {
	my ($self, $what, @args) = @_;
	#print STDERR "get $what @args ... ";
	if (exists $self->{$what}) {
		#print STDERR "from read file\n";
		if ($what eq 'config') {
			return $self->{'config'}{$args[0]};
		}
		return $self->{$what}{$args[0]}{$args[1]};
	}
	my $erg = $self->{'confhash'}{join(' ',$what,@args)};
	#print STDERR "from cache: $erg\n";
	return $erg;
}

sub set {
	my ($self, $key, $value) = @_;
	# if self->cachefile exists a config cache was read or written;
	return unless exists $self->{cachefile};
	return if exists $self->{is_writeable} && !$self->{is_writeable};
	unless ($self->{is_writeable}) {
		delete $self->{confhash}; # will 'untie' confhash
		my %confhash;
		# 'retie' cache writeable
		if (tie(%confhash, 'AnyDBM_File', $self->{cachefile}, O_RDWR|O_CREAT, 0600)) {
			$self->{confhash} = \%confhash;
			$self->{is_writeable} = 1;
		} else {
			$self->{is_writeable} = 0;
			return;
		}
	}
	$self->{confhash}{$key} = $value;
}

sub build_value($$$) {
	my $pre = $_[0]->{target}{'^'}{$_[1]};
	my $post = $_[0]->{target}{'$'}{$_[1]};
	my $val = ($pre ? $pre.' ' : '')
		. $_[2]
		. ($post ? ' '.$post : '');
	return $val;
}

# read the mrtg.cfg file
sub read_mrtg_config($$)
{
	my ($obj, $cfgfile) = @_;
	my ($opt, $tar, $val);
	my $line = '';
	my @lines;
	my (%targets, %options, @sorted, %config);
	$obj->{target} = \%targets;
	$obj->{option} = \%options;
	$obj->{sorted} = \@sorted;
	$obj->{config} = \%config;
	open(CFG, '<'.$cfgfile) || return ("Cannot open config file: $!");
	while(<CFG>) {
		next if /^\s*(\#|\z)/o; # ignore comment/empty lines
		s/\s+/ /go;	# collapse white space to one space
		s/ \z//o;	# remove newline
		if (/^ (.*)$/o) {	# continuation lines
			$lines[$#lines] .= "\n".$1;
		} else {
			push @lines, $_;
		}
	}
	close CFG;
	# set some defaults
	my %defaults = (
		directory => '',
		suppress => '',
		interval => 5,
		xsize => 400,
		ysize => 100,
		withpeak => '',
		ylegend => 'Bytes per Second',
		legend1 => 'Incoming Traffic in Bytes per Second',
		legend2 => 'Outgoing Traffic in Bytes per Second',
		legend3 => '--CALC--',
		legend4 => '--CALC--',
		legendi => 'In:',
		legendo => 'Out:',
		shortlegend => '--CALC--',
		kmg => ',k,M,G,T,P',
		kilo => 1000,
		);
	my %defoptions = (
		'indexgraph' => 'daily.s',
		'graph daily.s' => '-1250m   now    300',
		'graphsize daily.s' => '250 100',
		# don't set a default graphsize so override rules work
		# 'graphsize' => '400 100',
		'graph daily' => '-2000m   now    300',
		'graph weekly' => '-12000m  now   1800',
		'graph monthly' => '-800h    now   7200',
		'graph yearly' => '-400d    now  86400',
		'graphtotal' => 'no',
		'logarithmic' => 'no',
		#'error pic' => undef,
		);
	%config = (
		writeexpires => 'yes',
		background => '#ffffff',
		interval => 5,
		icondir => '',
		columns => 1,
		refresh => 'yes',
		);
	%{$options{_}} = %defoptions;
	%{$targets{_}} = %defaults;
	%{$targets{'$'}} = (); # prepend and append 'target' are empty by default
	%{$targets{'^'}} = ();
	my $mrtg290msgprinted = 0;
	foreach (@lines) {
		if (/^([\d\w]+)\[(\S*)\] ?: ?(.*)$/so) {
			# a target config line
			($tar, $opt, $val) = (lc($2), lc($1), $3);
			$val = '' if !defined $val; # get around undef values
			if (!exists $targets{$tar}) {
				# first occurance of a target, copy defaults
				# (don't need to check for '_' as this exists in any case)
				push @sorted, $tar;
				foreach my $k (keys %{$targets{_}}) {
					$targets{$tar}{$k} = build_value($obj, $k, $targets{_}{$k});
				}
				# copy prefix-only settings
				foreach my $k (keys %{$targets{'^'}}) {
					$targets{$tar}{$k} = build_value($obj, $k, '')
						unless $targets{$tar}{$k};
				}
				# copy postfix-only settings. no need for build_value here
				foreach my $k (keys %{$targets{'$'}}) {
					$targets{$tar}{$k} = $targets{'$'}{$k}
						unless $targets{$tar}{$k};
				}
				%{$options{$tar}} = %{$options{_}};
				# copy options from ^ and $ (thx to Mike Fisher)
				map {$options{$tar}{$_} = 1}
					(keys %{$options{'^'}}, keys %{$options{'$'}});
				# set a default title if none given
				# (shouldn't happen as mrtg requires a title)
				$targets{$tar}{'title'} = $tar unless $targets{$tar}{'title'};
			}
			if ($tar eq '_' && $val eq '') {
				# a line "Command[_]:", resets default
				if ($defaults{$opt}) {
					# there is a default for this, set it
					$targets{'_'}{$opt} = $defaults{$opt};
				} else {
					# no default, delete from _
					delete $targets{'_'}{$opt};
				}
			} elsif ($opt eq 'options') {
				# "Options[...]: " - add values to a set of options
				# not sure about this: mrtg allows only one "Options" line,
				# so the defaults should probably be replaced not merged
				$val = lc($val);
				map {$options{$tar}{$_} = 1} split(/[,\s]+/o, $val);
			} elsif ($tar eq '$' || $tar eq '^') {
				$targets{$tar}{$opt} = $val;
			} else {
				# "Command[...]: ..."
				$targets{$tar}{$opt} = build_value($obj, $opt, $val);
			}
			next;
		} elsif (/^([\d\w]+) ?: ?(.*)$/so) {
			# global option
			($tar, $opt, $val) = (undef, lc($1), $2);
			$config{$opt} = $val;
			next;
		} elsif (/^\w+\*\w/o) {
			print STDERR scalar(localtime()),
				" MRTG/RRD 14all.cgi: mrtg-2.9 configuration setting in '$cfgfile' detected\n"
				unless $mrtg290msgprinted++;
			next;
		}
		$_ =~ s/</&gt;/g;
		print STDERR scalar(localtime())," MRTG/RRD 14all.cgi: unknown setting in '$cfgfile': '$_'\n";
	}
	foreach $tar (keys %targets) {
		next if $tar eq '_';
		# we need to replace '&nbsp;' in the legends as we print them via rrdtool
		foreach $opt (qw/legend1 legend2 legend3 legend4 legend5 legendi legendo ylegend shortlegend/) {
			if (exists $targets{$tar}{$opt}) {
				$targets{$tar}{$opt} =~ s'&nbsp;' 'go;   #'
				$targets{$tar}{$opt} =~ tr/\n/ /;
				# escape % in legendi,legendo and shortlegend
				$targets{$tar}{$opt} =~ s/%/%%/go if $opt =~ m/[io]/o;
				if ($options{$tar}{'bits'} and $targets{$tar}{$opt} eq $defaults{$opt}) {
					$targets{$tar}{$opt} =~ s/Bytes/Bits/go;
				}
			}
		}
		# add settings from userrdtool[...] to options
		if (exists $targets{$tar}{'userrdtool'}) {
			foreach my $line (split(/\n/, $targets{$tar}{'userrdtool'})) {
				if (index($line, ':') > 0) {
					# line contains a ':'
					($opt, $val) = ($line =~ m/^([^,]+) ?: ?(.*)$/o);
					$options{$tar}{$opt} = $val;
				} else {
					# no ':' just notice it's there
					$options{$tar}{$line} = 1;
				}
			}
		}
		if (defined $targets{$tar}{'shortlegend'} and 
				$targets{$tar}{'shortlegend'} eq '--CALC--') {
			$targets{$tar}{'shortlegend'} = ($options{$tar}{'bits'} ? 'b/' :'B/') .
				($options{$tar}{'perhour'} ? 'h' : $options{$tar}{'perminute'} ? 'min' : 's');
		}
	}
	# add settings from userrdtool to config
	if (exists $config{'userrdtool'}) {
		foreach my $line (split(/\n/, $config{'userrdtool'})) {
			if (index($line, ':') > 0) {
				# line contains a ':'
				($opt, $val) = ($line =~ m/^([^:]+) ?: ?(.*)$/o);
				$config{$opt} = $val;
			} else {
				# no ':' just notice it's there
				$config{$line} = 1;
			}
		}
	}
	if (exists $config{refresh} && $config{refresh}
			&& $config{refresh} !~ /^((no?)|(false)|0)$/i
			&& $config{refresh} !~ m/^\d*[1-9]\d*$/o) {
		$config{refresh} = $config{interval} * 60;
	}
	return 0;
}

# fix some settings. gets called directly after read_mrtg_config
sub fixconfig {
	my $obj = shift;
	#   make sure icondir ends with /
	#   (don't use $SL as this is part of a URL)
	$obj->{config}{icondir} .= '/' if $obj->{config}{icondir} !~ m|/$|;
	my $err;
	# mrtg-2.9.0 added htmldir/imagedir/logdir settings:
	if ($obj->{config}{workdir}) {
		# workdir overrides htmldir/imagedir/logdir
		$obj->{config}{imagedir} = $obj->{config}{logdir}
		  = $obj->{config}{htmldir} = $obj->{config}{workdir};
		$err = check_directory($obj, $obj->{config}{workdir});
	} else {
		$err = check_directory($obj, $obj->{config}{imagedir})
			if $obj->{config}{imagedir};
		$err ||= check_directory($obj, $obj->{config}{logdir})
			if $obj->{config}{logdir};
		# 14all currently doesn't use htmldir
	}
	return $err;
}

# check if a directory given in a setting exists and is accessible
sub check_directory($$;$)
{
	my ($obj, $dir, $rw) = @_;
	# does dir exists and is readable/writeable?
	unless (-d $dir and ($rw ? -w _ : -r _)) {
		my $direrror = <<"EOT";
It looks like there is an error in the configuration. Please contact the
administrator of this page at $obj->{author}[1]. She/He will find more information
about this problem in the web servers log file.
EOT
		$direrror .= "(DEBUG enabled) Cannot access directory <i>$dir</i>: $!"
			if $obj->{debug};
		printf STDERR "%s 14all: Cannot access directory '$dir'"
			." (configfile %s)\n",localtime(),$obj->{cfgfile};
		return $direrror;
	}
	return 0;
}

#### end of package OneForAllConfig
