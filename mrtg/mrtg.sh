#!/bin/sh

# Temp, Humidity, Channel1-7 OpMode
# 25.90, 42.10, 0, 0, 0, 0, 1, 1, 0

grep=/bin/grep
chown=/bin/chown
expr=/usr/bin/expr
wget=/opt/bin/wget
cut=/usr/bin/cut
mrtg=/opt/bin/mrtg
var=/www/mrtg/var
watt=10 # terraduino and nslug2

$wget -O $var/terraduino -q http://192.168.128.250/rrd.html

data=`$grep -v "^#" $var/terraduino`

echo "$data" | $cut -f 1 -d , > $var/terraduino.temp
echo 0 >> $var/terraduino.temp

echo "$data" | $cut -f 2 -d , > $var/terraduino.humidity
echo 0 >> $var/terraduino.humidity

channel0=`echo "$data" | $cut -d , -f 3`
if test $channel0 -eq 1; then
  w=70
  watt=`$expr $watt + $w`
  channel0=$w
fi
echo "$channel0" > $var/terraduino.channel1
echo 0 >> $var/terraduino.channel1

channel1=`echo "$data" | $cut -d , -f 4`
if test $channel1 -eq 1; then
  w=70
  watt=`$expr $watt + $w`
  channel1=$w
fi
echo "$channel1" > $var/terraduino.channel2
echo 0 >> $var/terraduino.channel2

channel2=`echo "$data" | $cut -d , -f 5`
if test $channel2 -eq 1; then
  w=39
  watt=`$expr $watt + $w`
  channel2=$w
fi
echo "$channel2" > $var/terraduino.channel3
echo 0 >> $var/terraduino.channel3

channel3=`echo "$data" | $cut -d , -f 6`
if test $channel3 -eq 1; then
  w=39
  watt=`$expr $watt + $w`
  channel3=$w
fi
echo "$channel3" > $var/terraduino.channel4
echo 0 >> $var/terraduino.channel4

channel4=`echo "$data" | $cut -d , -f 7`
if test $channel4 -eq 1; then
  w=15
  watt=`$expr $watt + $w`
  channel4=$w
fi
echo "$channel4" > $var/terraduino.channel5
echo 0 >> $var/terraduino.channel5

channel5=`echo "$data" | $cut -d , -f 8`
if test $channel5 -eq 1; then
  w=15
  watt=`$expr $watt + $w`
  channel5=$w
fi
echo "$channel5" > $var/terraduino.channel6
echo 0 >> $var/terraduino.channel6

echo "$watt" > $var/terraduino.watt
echo 0 >> $var/terraduino.watt

echo $watt >> $var/allwatt

mem=`echo "$data" | $cut -d , -f 10`
echo $mem >  $var/terraduino.mem
echo 8000 >> $var/terraduino.mem

uptime=`echo "$data" | $cut -d , -f 11`
echo $uptime >  $var/terraduino.uptime
echo 0       >> $var/terraduino.uptime

$mrtg --confcache-file $var/mrtg.ok /www/mrtg/etc/mrtg.cfg

cd /www/mrtg
for log in uptime mem temp strom humidity; do
	./14all.cgi "cfg=14all.cfg&log=$log&png=daily.s&small=1" > /dev/null 2>&1
done

$chown www $var/*
