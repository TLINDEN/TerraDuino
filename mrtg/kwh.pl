#!/opt/bin/perl

my $var = "/www/mrtg/var";
my $current = "$var/allwatt";
my $watt = 0;

open A, "<$current";
while (<A>) {
  chomp;
  $watt += $_;
}
close A;
# $watt is now all watts per day per second

# how many kw/h
my $kwh = $watt / 3600000;

open LOG, ">>$var/kwhlog.txt";
my $date = scalar localtime(time);
print LOG "$date $kwh kW/h";
close LOG;

open A, ">$current";
print A "";
close A;

