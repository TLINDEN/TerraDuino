#!/usr/bin/perl
use Data::Dumper;

my $tpldir = shift;

if (! $tpldir) {
  print STDERR "Usage: $0 <template dir>\n";
  exit 1;
}

opendir D, $tpldir or die "Can't open dir $tpldir: $!\n";
my @templates = grep { /^[^\.]/ && -f "$tpldir/$_" && /\.tpl$/} readdir(D);
closedir D;

my %tpls;

# load the files
foreach my $tpl (@templates) {
  open TPL, "<$tpldir/$tpl" or die "Can't open template file $tpldir/$tpl: $!\n";
  my $T = join '', <TPL>;
  $tpls{$tpl}->{raw} = $T;
}

# extract insert keys and mark func templates
foreach my $tpl (keys %tpls) {
  my @inserts = $tpls{$tpl}->{raw} =~ /<% \s*insert \s*([a-zA-Z0-9]+) \s*%>/gm;

  if (! @inserts) {
    # function template, no insert in it
    $tpls{$tpl}->{func} = 1;
  }
  else {
    # base template
    $tpls{$tpl}->{inserts} = \@inserts;
  }
}

# snip base tpls into blocks by inserts
foreach my $tpl (keys %tpls) {
  if (! $tpls{$tpl}->{func}) {
    my $numblocks = scalar @{$tpls{$tpl}->{inserts}};
    for (my $id=0; $id<$numblocks; $id++) {
      my $block = $tpls{$tpl}->{inserts}->[$id];
      my($left, $right) = split /<% \s*insert \s*$block \s*%>/s, $tpls{$tpl}->{raw}, 2;
      if ($id == 0) {
	# the first block
	$tpls{$tpl}->{block}->{$block}->{left}->{raw} = $left;
      }
      else {
	# middle or last block, remove previous blocks
	$left =~ s/^.*<% \s*insert \s*[a-zA-Z0-9]+ \s*%>//s;
	$tpls{$tpl}->{block}->{$block}->{left}->{raw} = $left;
      }

      if ($id == $numblocks - 1) {
	# last block
	$tpls{$tpl}->{block}->{$block}->{right}->{raw} = $right;
      }
      else {
	# first or middle block, ignore right, because it becomes next ones left
	$tpls{$tpl}->{block}->{$block}->{right}->{raw} = '';
      }
    }
  }
}

#
# OUTPUT

# open files
open H, ">$tpldir/$tpldir.h"   or die "Could not open $tpldir/$tpldir.h: $!\n";
#open C, ">$tpldir/$tpldir.cpp" or die "Could not open $tpldir/$tpldir.cpp: $!\n";

#print H qq(#include "WebServer.h"\n);
#print C qq(#include "$tpldir.h"\n);

my %pmems;
my %vars;

my $C;

foreach my $tpl (keys %tpls) {
  if ($tpls{$tpl}->{func}) {
    # declare structs and funcs
    my $name = $tpl;
    $name =~ s/\.tpl//;
    #print H qq(extern struct DATA_$name;\n);
    #print H qq/void tpl_$name(WebServer &server, DATA_$name data);\n/;

    # final func
    $C .= qq(\n);
    $C .= qq/void tpl_$name(WebServer &server, DATA_$name data) {\n/;

    # block content
    if ( $tpls{$tpl}->{raw} =~ /<% \s*extend \s*([a-zA-Z0-9]+\.tpl) \s*%>/) {
      my $base = $1;
      if (exists $tpls{$base}) {
	foreach my $block ( @{$tpls{$base}->{inserts}} ) {
	  &progmem($base, $block, "left");
	  if ( $tpls{$tpl}->{raw} =~ /<% \s*block \s*$block \s*%>(.*)<% \s*endblock \s*$block \s*%>/s ) {
	    my $blockcontent = $1;
	    &block($blockcontent, $name, $block);
	  }
	  &progmem($base, $block, "right");
	}
      }
      else {
	print STDERR "Error in template $tpl: base template $base doesn't exist!\n";
	exit 1;
      }
    }

    $C.= qq(}\n\n);# end
  }
}



# progmems
foreach my $id (keys %pmem) {
  chomp  $pmem{$id};
  print H "P($id) = $pmem{$id};\n";
}

print H $C;
close H;

#print Dumper(\%vars);exit;

# final output
print "Created $tpldir/$tpldir.h\n";
print "You should put the directory $tpldir into your 'libraries' folder\n";
print "and add the following code to your sketch to use it:\n\n";
print qq(
/*
 * Add this code to the top of your sketch
 */
#include "Ethernet.h"
#include "WebServer.h"\n\n);

foreach my $tpl (sort keys %vars) {
  print "struct DATA_$tpl {\n";
  foreach my $var (sort keys %{$vars{$tpl}}) {
    if ($vars{$tpl}->{$var} =~ /^array: (.*)/) {
      print "  DATA_$1 $var\[...\]; // fill in the appropriate array size\n";
    }
    else {
      print "  $vars{$tpl}->{$var} $var;\n";
    }
  }
  print "};\n";
}

print qq-
char endl = '\\n';

template<class T>
inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

#include "Templates.h"

/*
 * To call one of the generated template functions, do:
 */

WebServer webserver(PREFIX, 80);

-;

foreach my $tpl (sort keys %vars) {
  print "  DATA_$tpl ${tpl}_vars;\n";
  foreach my $var (sort keys %{$vars{$tpl}}) {
    print "  ${tpl}_vars.$var = ...; // fill with appropriate values\n";
  }
  print "\n";
  print "  tpl_index(webserver, ${tpl}_vars)\n";
}


sub block {
  my($content, $name, $block) = @_;
  return if($content =~ /^\s*$/s);
  my @lines = split /\n/, $content;
  my $loop = 0;
  my $loopcode = '';

  my $n = 1;
  foreach (@lines) {
    next if (/^\s*$/);
    s/"/\\"/g;
    s/^\s*//;
    chomp();
    if ($loop) {
      if (/<% \s*endfor \s*%>/) {
	$loop = 0;
	$n = &loop($loopcode, $name, $block, $n);
      }
      else {
	$loopcode .= $_ . "\n";
      }
      next;
    }
    if (/<% \s*([a-zA-Z0-9\*\-_]+) \s*[a-zA-Z0-9]+ \s*%>/) {
      # print directly
      s/<% \s*([a-zA-Z0-9\*\-_]+) \s*([a-zA-Z0-9]+) \s*%>/$variable = $2; $type = $1; $vars{$name}->{$variable} = $type; "\" << data.$variable << \""/ge;
      $C .= qq/  server << "$_" << endl;\n\n/;
    }
    elsif (/<% \s*for \s*.*%>/) {
      $loop = 1;
      $loopcode = $_ . "\n";
    }
    else {
      my $id = "${name}_${block}_${n}";
      $pmem{$id} = "\"$_\"";
      $C.= qq/  server.printP($id);\n\n/;
    }
    $n++;
  }
}

sub loop {
  my ($loopcode, $name, $block, $n) = @_;
  my ($start, @content) = split /\n/, $loopcode;
  if ($start =~ /<% \s*for \s*([a-zA-Z0-9]+) \s*in \s*([a-zA-Z0-9\*\-_]+) \s*([a-zA-Z0-9]+) \s*%>/) {
    my $member = $1;
    my $type   = $2;
    my $array  = $3;
    $vars{$name}->{$array} = "array: $type";

    $C .= "  for(int i = 0; i < (sizeof(data.$array) / sizeof(DATA_$type)); i++) {\n";

    foreach (@content) {
      if (/<% \s*([a-zA-Z0-9\*\-_]+) \s*[a-zA-Z0-9]+ \s*%>/) {
	# print directly
	s/<% \s*([a-zA-Z0-9\*\-_]+) \s*([a-zA-Z0-9]+) \s*%>/$variable = $2; $type = $1; $vars{$name}->{$variable} = $type; "\" << data.$variable << \""/ge;
	$C .= qq/    server << "$_" << endl;\n\n/;
      }
      if (/<% \s*([a-zA-Z0-9\*\-_]+) \s*[a-zA-Z0-9\.]+ \s*%>/) {
	# print directly , member var
	# type member name
	s/<% \s*([a-zA-Z0-9\*\-_]+) \s*([a-zA-Z0-9]+)\.([a-zA-Z0-9]+) \s*%>/$variable = $3; $vtype = $1; $vars{$type}->{$variable} = $vtype; "\" << data.$array\[i\].$variable << \""/ge;
	$C .= qq/    server << "$_" << endl;\n\n/;
      }
      else {
	my $id = "${name}_${block}_${n}";
	$pmem{$id} = "\"$_\"";
	$C.= qq/  server.printP($id);\n\n/;
      }
      $n++;
    }
    $C .= "  }\n";
  }

  return $n;
}

sub progmem {
  my ($tpl, $block, $lr) = @_;
  return if($tpls{$tpl}->{block}->{$block}->{$lr}->{raw} =~ /^\s*$/s);

  my @lines = split /\n/, $tpls{$tpl}->{block}->{$block}->{$lr}->{raw};
  my $name = $tpl;
  $name =~ s/\.tpl//;

  my $id = "${name}_${block}_${lr}";

  $pmem{$id} = "\n";

  foreach (@lines) {
    next if (/^\s*$/);
    s/"/\\"/g;
    s/^\s*//;
    chomp();
    $pmem{$id} .= "  \"$_\"\n";
  }

  $C .= qq/  server.printP($id);\n\n/;
}

#print Dumper(\%tpls);
