/* -*-c-*-
 * Terraduino Terrarium Controller with Arduino.
 *
 * Copyright (c) 2012 Thomas Linden
 *
 */

#include <MemoryFree.h>

/* Sunrise times stored as progmem table */
#include "Flash.h"
#include "Sunrise.h"
#include "Messages.h"

/* sensor lib */
#include "DHT.h"

/* eeprom lib */
#include <EEPROM.h>
#include "EEany.h"

/* RTC Timer access */
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h> 
#include <inttypes.h>

/* Webserver */
#include "SPI.h"
#include "avr/pgmspace.h"
#include "Ethernet.h"
#include "WebServer.h"

/* atoi ... */
#include <stdio.h>

/* watchdog (http://tushev.org/articles/electronics/48-arduino-and-watchdog-timer) */
#include <avr/wdt.h>

/* defines */
#define SUNRISE 0
#define STATIC  1
#define MANUAL  2
#define DHTPIN 25
#define DHTTYPE DHT22 
#define NOTE_C4  262
#define WEBDUINO_FAIL_MESSAGE "<h1>Request Failed</h1>"
#define PREFIX ""
#define MAXBYTES 64
#define NOTE_A 1136
#define NOTE_B 1014
#define NOTE_C 1915
#define NOTE_D 1700
#define NOTE_E 1519
#define NOTE_F 1432
#define NOTE_G 1275
#define PINON  'X'
#define PINOFF '-'
#define MAXUP 86400 // after 1 day uptime keep the value

/* global vars */
time_t t = 0;
time_t booted = 0;
long uptime = 0;
int startdelay = 0;
int stopdelay = 0;
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const char endl = '\n';
bool manual = false;
uint8_t mainstatus = 0;
uint8_t pressed    = 0;
uint8_t swpressed  = 0;
long timer = 0;
long moment = 0;
long sttimer = 0;
long stmoment = 0;
long airtimer = 0;
long airmoment = 0;
bool airon = false;
long alarmpaused = 0;

const int alarmwait         = 120; // min, 2 hours
const long timerintervall   = 5000; //orig: 500;
const long airtimerinterval = 72000; // 10 min

uint8_t channel = 0;
uint8_t program = 0;
bool INIT = true;
bool RUN  = false;
uint8_t runtime = 0;
long cooldown[] = {0, 0, 0, 0, 0, 0};
int begin = 0;
int end = 0;

// serial parser vars
int     value[4] = { 0 };
uint8_t nvalue = 0;
char    buffer[5] = { 0 };
byte    idx = 0;

    
uint8_t onebyte = 0;
uint8_t command = 0;
char parameter[MAXBYTES] = { 0 };
byte index = 0;
bool parametermode = false;

/* important for EEany initialization */
int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1001;

/* post parser vars */
char parsename[32] = { 0 };
char parsevalue[32] = { 0 };
bool parsing = true;

/* PINs */
const uint8_t switches[] = { 15, 49, 47, 45, 43, 41 };
const uint8_t leds[]     = { 16, 48, 46, 44, 42, 40 };
const uint8_t relays[]   = { 36, 34, 32, 30, 28, 26, 22 };
uint8_t state[]          = { 0, 0, 0, 0, 0, 0 };
uint8_t runstate[]       = { 0, 0, 0, 0, 0, 0 };

/* names for the channels */
const String names[7] = { "70 W Links", "70 W Rechts", "40 W Vorn", "40 W Hinten", "15 W Links", "15 W Rechts", "Reserve"};

const uint8_t air        = 24;
const uint8_t statusled  = 27;
const uint8_t speaker    = 17;
const uint8_t mainswitch = 19;
const uint8_t mainled    = 18;

const uint8_t numchannels = 7;
const uint8_t numswitches = 6;
uint8_t i = 0;

const uint8_t dayspermonth[] = {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* global objects */
DHT dht(DHTPIN, DHTTYPE);
Database db;
WebServer webserver(PREFIX, 80);

/* used to store http post/get vars  and to fill template functions */
struct POST {
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  int i6;
  int i7;
  int i8;
  int i9;
  int i10;
  int i11;
  int i12;
  char s1[64];
} post;



/*
 * We are not using the internal tone()
 * function because it just doesn't work in my setup,
 * for whatever strange reasons.
 *
 * Code taken from: https://github.com/rynet/Arduino-Tone-Library/blob/master/toneLibrary.pde
 */
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speaker, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speaker, LOW);
    delayMicroseconds(tone);
  }
}

/***********************************/



/*
 * Webserver output functions
 */
 
void prog2web(int p, WebServer &server) {
  // print a human readable version of the given program
  
  server << f_shc_pn << p << ' ';
  
  if(db.programs[p].type == STATIC) {
    server << f_shc_st << f_shc_start;
    server << N(db.programs[p].start_hour) << ':' << N(db.programs[p].start_min);
    server << bis << f_shc_stop;
    server << N(db.programs[p].stop_hour)  << ':' << N(db.programs[p].stop_min);
  }
  else if(db.programs[p].type == MANUAL) {
    server << f_shc_man;
  }
  else {
    server << f_shc_ast << f_shc_stdel << db.programs[p].start_delay << f_shc_min;
    server << bis << f_shc_stodel << db.programs[p].stop_delay << f_shc_min;
  }
  if(db.programs[p].cooldown > 0) {
    server << bis << f_shc_delay << db.programs[p].cooldown << f_shc_min;
  }
}



void tpl_index(WebServer &server) {
  server << hhead << thome << hmenu << ch1;
  server << table;
  server << tra << tdr << itemp << tde << td << post.i1 << igrad << tde << tre;
  server << tra << tdr << ihum  << tde << td << post.i2 << iperc << tde << tre;
  server << tra << tdr << itime << tde << td << N(post.i3) << '.' << N(post.i4) << '.' << post.i5;
  server << ' ' << N(post.i6) << ':' << N(post.i7) << ':' << N(post.i8) << f_shc_clock << tde << tre;
  
  server << tra << tdr << imode << tde << td ;
  if(manual) {
    server << imm;
  }
  else {
    server << ima;
  }
  server << tde << tre;

  server << tra << tdr << isunr << tde << td << N(post.i9)  << ':' << N(post.i10) << f_shc_clock << tde << tre;
  server << tra << tdr << isuns << tde << td << N(post.i11) << ':' << N(post.i12) << f_shc_clock << tde << tre;
  
  server << tra << tdrt << ichan << tde << td;
  t = gettimeofday();
  begin = getsunrise(t);
  delay(1);
  end   = getsunset(t);
  server << tablesm;
  for (channel=0; channel<numchannels; channel++) {
    server << tra << td << chlinkl << channel << chlinkr << names[channel] << chlinke << ':' << tde << td;
    if(manual ||Â db.programs[db.channels[channel].program].type == MANUAL) {
      if(state[channel] == HIGH) {
        server << irun;
      }
      else {
        server << ioff;
      }
    }
    else {
      if(operate(channel, t)) {
        server << irun;
      }
      else {
        server << ioff;
      }
    }
    server << tde << td;
    server << prlinkl << db.channels[channel].program << chlinkr;
    if(db.programs[db.channels[channel].program].type == MANUAL) {
      server << f_shc_man; 
    }
    else if(db.programs[db.channels[channel].program].type == STATIC) {
      server << f_shc_st << ' ' << N(db.programs[db.channels[channel].program].start_hour) << ':' << N(db.programs[db.channels[channel].program].start_min);
      server << f_shc_clock << bis << N(db.programs[db.channels[channel].program].stop_hour) << ':' << N(db.programs[db.channels[channel].program].stop_min);
      server << f_shc_clock;
    }
    else {
      server << f_shc_ast << ' ';
      int B = begin + db.programs[db.channels[channel].program].start_delay + elapsed_sleep_days(t, channel);
      int E = end   - (db.programs[db.channels[channel].program].stop_delay  + elapsed_sleep_days(t, channel));
      if(B > E) {
        server << N(0) << ':' << N(0) << f_shc_clock << bis << N(0) << ':' << N(0) << f_shc_clock;
      }
      else {
        server << N((B - (B % 60)) / 60); // start hour
        server << ':';
        server << N(B % 60);
        server << f_shc_clock << bis;
        server << N((E - (E % 60)) / 60); // stop hour
        server << ':';
        server << N(E % 60) << f_shc_clock;
      }
    }
    if(db.programs[db.channels[channel].program].type != MANUAL) {
      if( db.programs[db.channels[channel].program].cooldown > 0 ) {
        server << f_shc_delay << db.programs[db.channels[channel].program].cooldown;
        if(db.programs[db.channels[channel].program].cooldown == 1) {
          server << f_shc_1min;
        }
        else {
           server << f_shc_min;
        }
      }
    }
    server << chlinke << tde << tre;
  }
  server << tablee << tde << tre;
 
  server << tra << tdr << iklim << tde << td;
  if(db.settings.aircondition) {
    server << airactive;
    server << bis;
    if(airon) {
      server << irun;
    }
    else {
      server << ioff;
    }
    server << '(' << db.settings.air_tmin << igrad << bis << db.settings.air_tmax << igrad << ')';
  }
  else {
    server << airinactive;
  }
  server << tde << tre;
  
  server << tra << tdr << lastboot << tde << td << N(day(booted)) << '.' << N(month(booted)) << '.' << year(booted);
  server << ' ' << N(hour(booted)) << ':' << N(minute(booted)) << ':' << N(second(booted)) << f_shc_clock << tde << tre;
  
  server << tablee << hfoot;
}

void tpl_channels(WebServer &server) {
  server << hhead << tchannels << hmenu << chc;
  server << tablechan;

  for(channel=0; channel<numchannels; channel++) {
    server << tra << td;
    server << chlinkl << channel << chlinkr << names[channel] << chlinke << tde;
    server << td;
    prog2web(db.channels[channel].program, server);
    server << tde;
  }
  server << tre << tablee << hfoot;
}

void tpl_setdate(WebServer &server) {
  server << hhead << tdate << hmenu << chd;
  server << msg1 << post.s1 << msg2;
  server << sdform << tablesd;

  server << tra;

  server << sdfi2 << 0 << sdfv << N(post.i1) << sdfe << tde;
  server << sdfi2 << 1 << sdfv << N(post.i2) << sdfe << tde;
  server << sdfi4 << 2 << sdfv << post.i3    << sdfe << tde;
  server << sdfi2 << 3 << sdfv << N(post.i4) << sdfe << tde;
  server << sdfi2 << 4 << sdfv << N(post.i5) << sdfe << tde;
  server << sdfi2 << 5 << sdfv << N(post.i6) << sdfe << tde;

  server << tre << tra;

  server << sdfdst << post.i7 << sdfe << sdfdsth << tde << tre;

  server << tablee << submit << forme << hfoot;
}

void tpl_setprogram(WebServer &server) {
  server << hhead << tsetp << hmenu << chs;
  server << msg1 << post.s1 << msg2;
  
    server << spform;
    server << sdchannel << names[post.i1] << br;
    server << hidden << post.i1 << sdfe;
    server << sdpc << spsel;
    for(program = 0; program<maxprograms; program++) {
      server << opt << program << '"';
      if(program == db.channels[post.i1].program) {
        server << selected;
      }
      server << '>';
      prog2web(program, server);
      server << opte;
    }
    server << sele << br << submit << forme;
  
  server << hfoot;
}

void tpl_programs(WebServer &server) {
  server << hhead << tprog << hmenu << chp;
  server << msg1 << post.s1 << msg2;
  server << tableprog;
  for(program = 0; program<maxprograms; program++) {
    server << tra << td;
    server << prlinkl << program << chlinkr << program << chlinke << td;
    prog2web(program, server);
    server << tde << tre;
  }
  server << tablee << hfoot;
}

void tpl_editprogram(WebServer &server) {
  server << hhead << tprogset << hmenu << chps;
  server << msg1 << post.s1 << msg2;
  server << spsform << f_shc_pr << post.i1;
  server << hidden << post.i1 << sdfe;

  server << tableprogset;
  server << spsel;

  for (i=0; i<3; i++) {
    server << opt << i << '"';
    if(db.programs[post.i1].type == i) {
      server << selected;
    }
    server << '>';
    if(i == STATIC) {
      server << f_shc_st;
    }
    else if(i == MANUAL) {
      server << f_shc_man;
    }
    else {
      server << f_shc_ast;
    }
    server << opte;
  }
  server << sele << tde << tre;

  // start 1:hour.i2 2:minute.i3
  server << tra << tdr << f_shc_start << tde << td;
  server << spf2 << 2 << sdfv << N(db.programs[post.i1].start_hour) << sdfe << ':';
  server << spf2 << 3 << sdfv << N(db.programs[post.i1].start_min)  << sdfe << f_shc_clock;
  server << tde << tre;

  // stop 3:hour.i4 4:minute.i5
  server << tra << tdr << f_shc_stop << tde << td;
  server << spf2 << 4 << sdfv << N(db.programs[post.i1].stop_hour) << sdfe << ':';
  server << spf2 << 5 << sdfv << N(db.programs[post.i1].stop_min)  << sdfe << f_shc_clock;
  server << tde << tre;

  // startdelay 5:startdelay.i6
  server << tra << tdr << f_shc_stdel << tde << td;
  server << spf5 << 6 << sdfv << db.programs[post.i1].start_delay << sdfe << f_shc_min;
  server << tde << tre;

  // startdelay 6:startdelay.i7
  server << tra << tdr << f_shc_stodel << tde << td;
  server << spf5 << 7 << sdfv << db.programs[post.i1].stop_delay << sdfe << f_shc_min;
  server << tde << tre;

  // cooldown 7:cooldown.i8
  server << tra << tdr << spcooldown << tde << td;
  server << spf5 << 8 << sdfv << db.programs[post.i1].cooldown << sdfe << f_shc_min;
  server << tde << tre;
  
  // sleepday
  server << tra << tdr << spsleep << tde << td;
  server << spf5 << 9  << sdfv << N(db.programs[post.i1].sleep_day) << sdfe << '.';
  server << spf5 << 10 << sdfv << N(db.programs[post.i1].sleep_mon) << sdfe;
  server << tde << tre;
  
  server << tra << tdr << spsleepincr << tde << td;
  server << spf5 << 11 << sdfv << N(db.programs[post.i1].sleep_increment) << sdfe << f_shc_min;
  server << tde << tre;

  server << tablee << submit << forme << hfoot;
}

void tpl_setip(WebServer &server) {
  server << hhead << tip << hmenu << chip;
  server << msg1 << post.s1 << msg2;
  server << ipform;
  server << spf3 << 0 << sdfv << db.settings.octet1 << sdfe << '.';
  server << spf3 << 1 << sdfv << db.settings.octet2 << sdfe << '.';
  server << spf3 << 2 << sdfv << db.settings.octet3 << sdfe << '.';
  server << spf3 << 3 << sdfv << db.settings.octet4 << sdfe;
  server << ipgw;
  server << spf3 << 4 << sdfv << db.settings.gw1 << sdfe << '.';
  server << spf3 << 5 << sdfv << db.settings.gw2 << sdfe << '.';
  server << spf3 << 6 << sdfv << db.settings.gw3 << sdfe << '.';
  server << spf3 << 7 << sdfv << db.settings.gw4 << sdfe;
  server << br << submit << forme << hfoot;
}

void tpl_setair(WebServer &server) {
  server << hhead << tair << hmenu << chair;
  server << msg1 << post.s1 << msg2;
  server << airform << tableair;

  server << spsel;

  server << opt << 0 << '"';
  if(db.settings.aircondition == 0) {
    server << selected;
  }
  server << '>' << airinactive << opte;

  server << opt << 1 << '"';
  if(db.settings.aircondition == 1) {
    server << selected;
  }
  server << '>' << airactive << opte;
  server << tde << tre;

  server << tra << tdr << tmin << tde;
  server << td << spf2 << 2 << sdfv << db.settings.air_tmin << sdfe << igrad << tde << tre;

  server << tra << tdr << tmax << tde;
  server << td << spf2 << 3 << sdfv << db.settings.air_tmax << sdfe << igrad << tde << tre;
  
  server << tra << tdr << talarm << tde;
  server << td << spf2 << 4 << sdfv << db.settings.air_alarm << sdfe << igrad << tde << tre;

  server << tablee << submit << forme << hfoot;
}

/*
 * Webserver display functions
 */

void resetpost() {
 post.i1 = 0;
 post.i2 = 0;
 post.i3 = 0;
 post.i4 = 0;
 post.i5 = 0;
 post.i6 = 0;
 post.i7 = 0;
 post.i8 = 0;
 post.i9 = 0;
 post.i10 = 0;
 post.i11 = 0;
 post.i12 = 0;
 post.s1[0] = '\0';
 parsename[0] = '\0';
 parsevalue[0] = '\0';
 parsing = true;
}

void www_home(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  resetpost();
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }

  server.httpSuccess();

  if (type == WebServer::GET) {
    t  = gettimeofday();
    int begin = getsunrise(t);
    int end   = getsunset(t);
    
    post.i1 = temperature();
    post.i2 = humidity();
    
    post.i3 = day(t);
    post.i4 = month(t);
    post.i5 = year(t);
    
    post.i6 = hour(t);
    post.i7 = minute(t);
    post.i8 = second(t);

    post.i9  = (begin - (begin % 60)) / 60;
    post.i10 = begin % 60;
    post.i11 = (end - (end % 60)) / 60;
    post.i12 = end % 60;
    
    tpl_index(server);
  }
}
  

void www_channels(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }
  server.httpSuccess();
  tpl_channels(server);
}

void www_setdate(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  resetpost();
  t  = gettimeofday();

  server.httpSuccess();

  if (type == WebServer::POST)  {
    // initialize them, so that setTime() doesnt get undef's
    // in case the user didn't post them all
    post.i1 = day(t);
    post.i2 = month(t);
    post.i3 = year(t);
    post.i4 = hour(t);
    post.i5 = minute(t);
    post.i6 = second(t);
    post.i7 = dst(t);
    
    while (parsing) {
      parsing = server.readPOSTparam(parsename, 32, parsevalue, 32);
      if (strcmp(parsename, "0")  == 0){post.i1 = atoi(parsevalue); }
      if (strcmp(parsename, "1")  == 0){post.i2 = atoi(parsevalue); }
      if (strcmp(parsename, "2")  == 0){post.i3 = atoi(parsevalue); }
      if (strcmp(parsename, "3")  == 0){post.i4 = atoi(parsevalue); }
      if (strcmp(parsename, "4")  == 0){post.i5 = atoi(parsevalue); }
      if (strcmp(parsename, "5")  == 0){post.i6 = atoi(parsevalue); }
      if (strcmp(parsename, "6")  == 0){post.i7 = atoi(parsevalue); }
    }
    
    int error = 0;
    if(post.i1 > 31) {
      error = 1;
      err_day.copy(post.s1);
    }
    if(post.i2 > 12) {
      error = 1;
      err_mon.copy(post.s1);
    }
    if(post.i3 > 3600) {
      error = 1;
      err_yea.copy(post.s1);
    }
    if(post.i4 > 23) {
      error = 1;
      err_hour.copy(post.s1);
    }
    if(post.i5 > 59) {
      error = 1;
      err_min.copy(post.s1);
    }
    if(post.i6 > 59) {
      error = 1;
      err_sec.copy(post.s1);
    }
    
    if(error) {
      tpl_setdate(server);
      return;
    }
    
    // set the time, first run, using raw input
    setTime(post.i4, post.i5, post.i6, post.i1, post.i2, post.i3);
    t = now();

    if(post.i7 == 1) {
      // user supplied time is within dst, so substract 60 minutes
      // and set the corrected time again, since we run internally
      // ALWAYS within winter time
      t -= 3600;
    }

    setTime(t);
    RTC.set(t);
    
    sddone.copy(post.s1); 
  }

  t  = gettimeofday();
  post.i1 = day(t);
  post.i2 = month(t);
  post.i3 = year(t);
  post.i4 = hour(t);
  post.i5 = minute(t);
  post.i6 = second(t);
  post.i7 = dst(t);

  tpl_setdate(server);
}


void www_setip(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  resetpost();
  server.httpSuccess();
  if (type == WebServer::POST)  {
    while (parsing) {
      parsing = server.readPOSTparam(parsename, 32, parsevalue, 32);
      if (strcmp(parsename, "0") == 0){post.i1 = atoi(parsevalue);}
      if (strcmp(parsename, "1") == 0){post.i2 = atoi(parsevalue);}
      if (strcmp(parsename, "2") == 0){post.i3 = atoi(parsevalue);}
      if (strcmp(parsename, "3") == 0){post.i4 = atoi(parsevalue);}
      if (strcmp(parsename, "4") == 0){post.i5 = atoi(parsevalue);}
      if (strcmp(parsename, "5") == 0){post.i6 = atoi(parsevalue);}
      if (strcmp(parsename, "6") == 0){post.i7 = atoi(parsevalue);}
      if (strcmp(parsename, "7") == 0){post.i8 = atoi(parsevalue);}
    }

    if(post.i1 < 1 || post.i1 > 255 || post.i2 > 255 || post.i3 > 255 || post.i4 > 255 || post.i5 < 1 || post.i5 > 255 || post.i6 > 255 || post.i7 > 255 || post.i8 > 255) {
      err_ip.copy(post.s1);
      tpl_setip(server);
      return;
    }

    Settings S = db.settings;
    S.octet1 = post.i1;
    S.octet2 = post.i2;
    S.octet3 = post.i3;
    S.octet4 = post.i4;
    S.gw1    = post.i5;
    S.gw2    = post.i6;
    S.gw3    = post.i7;
    S.gw4    = post.i8;
    ee_wr_settings(S);
    ipdone.copy(post.s1);
    db = ee_getdb();
  }

  tpl_setip(server);
}


void www_setprogram(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  // 0 = i1 = channel, 1 = i2 = program
  resetpost();
  server.httpSuccess();

  if (type == WebServer::GET)  {
    URLPARAM_RESULT rc;
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
	rc = server.nextURLparam(&url_tail, parsename, 32, parsevalue, 32);
	if (strcmp(parsename, "channel") == 0){post.i1 = atoi(parsevalue); Serial << "uri chan: " << post.i1 << endl;}
      }
      if(post.i1 < 0 || post.i1 > 6) {
        err_unknchan.copy(post.s1);
	tpl_setprogram(server);
	return;
      }
    }
    else {
      err_unknid.copy(post.s1);
      tpl_setprogram(server);
      return;
    }
  }

  // 0 = i1 = channel, 1 = i2 = program
  if (type == WebServer::POST)  {
    while (parsing) {
      parsing = server.readPOSTparam(parsename, 32, parsevalue, 32);
      if (strcmp(parsename, "0") == 0){post.i1 = atoi(parsevalue);}
      if (strcmp(parsename, "1") == 0){post.i2 = atoi(parsevalue);}
    }

    if(post.i2 < 0 || post.i2 > 31) {
      err_unknpr.copy(post.s1);
      tpl_setprogram(server);
      return;
    }
    else {
      Channel c = db.channels[post.i1];
      c.program = post.i2;
      ee_wr_channel(c);
      spdone.copy(post.s1);
      db = ee_getdb();
    }
  }

  tpl_setprogram(server);
}



void www_programs(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  resetpost();
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }
  server.httpSuccess();
  if (type == WebServer::GET)  {
    tpl_programs(server);
  }
}


void www_editprogram(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  resetpost();
  server.httpSuccess();
  if (type == WebServer::GET)  {
    URLPARAM_RESULT rc;
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
	rc = server.nextURLparam(&url_tail, parsename, 32, parsevalue, 32);
	if (strcmp(parsename, "program") == 0){post.i1 = atoi(parsevalue);}
      }
      if(post.i1 < 0 || post.i1 > 31) {
        err_unknprog.copy(post.s1);
	tpl_editprogram(server);
	return;
      }
    }
    else {
      err_unknprogid.copy(post.s1);
      tpl_editprogram(server);
      return;
    }
  }

  if (type == WebServer::POST)  {
    while (parsing) {
      parsing = server.readPOSTparam(parsename, 32, parsevalue, 32);
      if (strcmp(parsename, "0")  == 0){post.i1  = atoi(parsevalue);}
      if (strcmp(parsename, "1")  == 0){post.i2  = atoi(parsevalue);}
      if (strcmp(parsename, "2")  == 0){post.i3  = atoi(parsevalue);}
      if (strcmp(parsename, "3")  == 0){post.i4  = atoi(parsevalue);}
      if (strcmp(parsename, "4")  == 0){post.i5  = atoi(parsevalue);}
      if (strcmp(parsename, "5")  == 0){post.i6  = atoi(parsevalue);}
      if (strcmp(parsename, "6")  == 0){post.i7  = atoi(parsevalue);}
      if (strcmp(parsename, "7")  == 0){post.i8  = atoi(parsevalue);}
      if (strcmp(parsename, "8")  == 0){post.i9  = atoi(parsevalue);}
      if (strcmp(parsename, "9")  == 0){post.i10 = atoi(parsevalue);}
      if (strcmp(parsename, "10") == 0){post.i11 = atoi(parsevalue);}
      if (strcmp(parsename, "11") == 0){post.i12 = atoi(parsevalue);}
    }

    int error = 0;
    if(post.i2 < 0 || post.i2 > 2) {
      err_type.copy(post.s1);
      error = 1;
    }
    if(post.i3 < 0 || post.i3 > 23) {
      err_hour1.copy(post.s1);
      error = 1;
    }
    if(post.i4 < 0 || post.i4 > 59) {
      err_min1.copy(post.s1);
      error = 1;
    }
    if(post.i5 < 0 || post.i5 > 23) {
      err_hour2.copy(post.s1);
      error = 1;
    }
    if(post.i6 < 0 || post.i6 > 59) {
      err_min2.copy(post.s1);
      error = 1;
    }
    if(post.i7 < 0 || post.i7 > 65536) {
      err_delay1.copy(post.s1);
      error = 1;
    }
    if(post.i8 < 0 || post.i8 > 65536) {
      err_delay1.copy(post.s1);
      error = 1;
    }
    if(post.i9 < 0 || post.i9 > 65536) {
      err_cooldown.copy(post.s1);
      error = 1;
    }
    if(post.i10 > 0 && post.i11 > 0) {
      if(post.i10 < 1 || post.i10 > 31) {
        err_sleepday.copy(post.s1);
        error = 1;
      }
      if(post.i11 < 1 || post.i11 > 12) {
        err_sleepmon.copy(post.s1);
        error = 1;
      }
      if(post.i12 < 1 || post.i12 > 120) {
        err_sleepincr.copy(post.s1);
        error = 1;
      }
    }
    else {
     // sanitize sleep day data
     post.i10 = 0;
     post.i11 = 0;
     post.i12 = 0; 
    }
    
    if(! error) {
      Program p = db.programs[post.i1];
      p.type       = post.i2;
      p.start_hour = post.i3;
      p.start_min  = post.i4;
      p.stop_hour  = post.i5;
      p.stop_min   = post.i6;
      p.start_delay= post.i7;
      p.stop_delay = post.i8;
      p.cooldown   = post.i9;
      p.sleep_day  = post.i10;
      p.sleep_mon  = post.i11;
      p.sleep_increment  = post.i12;
      ee_wr_program(p);
      progdone.copy(post.s1);
      delay(10);
      db = ee_getdb();
    }
  }

  tpl_editprogram(server);
}



void www_air(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  resetpost();
  server.httpSuccess();

  if (type == WebServer::POST)  {
    while (parsing) {
      parsing = server.readPOSTparam(parsename, 32, parsevalue, 32);
      if (strcmp(parsename, "1")  == 0){post.i1 = atoi(parsevalue);}
      if (strcmp(parsename, "2")  == 0){post.i2 = atoi(parsevalue);}
      if (strcmp(parsename, "3")  == 0){post.i3 = atoi(parsevalue);}
      if (strcmp(parsename, "4")  == 0){post.i4 = atoi(parsevalue);}
    }

    if(post.i2  < 0 || post.i2  > 65 || post.i3 < 0 || post.i3 > 65 || post.i4 < 0 ||Â post.i4 > 65) {
      err_air.copy(post.s1);
      tpl_setair(server);
      return;
    }
    else {
      Settings S = db.settings;
      if(post.i1) {
	S.aircondition = true;
      }
      else {
	S.aircondition = false;
      }
      S.air_tmin = post.i2;
      S.air_tmax = post.i3;
      S.air_alarm= post.i4;
      
      ee_wr_settings(S);
      airdone.copy(post.s1);
      db = ee_getdb();
    }
  }
  tpl_setair(server);
}


void www_rrd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  server.httpSuccess();
  server << f_rrd << endl;
  server << temperature() << sep << humidity();
  if(manual) {
    for(channel=0; channel<numswitches; channel++) {
      server << sep << state[channel];
    }
    server << sep << 0;
  }
  else {
    for(channel=0; channel<numchannels; channel++) {
      if(db.programs[db.channels[channel].program].type == MANUAL) {
        server << sep << state[channel];
      }
      else {
        server << sep << operate(channel, t);
      }
    }
  }
  server << sep << freeMemory();
  server << sep << uptime;
  server << endl;
  delay(100);
}

/*
 * Helper functions
 */
bool isleafyear(uint8_t Y) {
  /* tell if current year is a leaf year */
  if ((Y % 400) == 0)
    return true;
  else if ((Y % 100) == 0)
    return false;
  else if ((Y % 4) == 0)
    return true;

  return false;
}  

uint8_t getdayspermonth(uint8_t M, uint8_t Y) {
  /* how many days has the current month */
  if (M == 2)   {
    if (isleafyear(Y))
      return 29;
    else
      return 28;
  }

  if ((M >= 1) && (M <= 12)) {
    return dayspermonth[M];
  }
  else {
    return 0;
  }
} 

int getdayofyear(uint8_t T, uint8_t M, int Y) {
  /* the how many'th day of year is the current day */
  if ((M == 0) || (M > 12)) {
    return -1;
  }

  int     LT = T;
  uint8_t LM = M;

  while (LM > 1) {
    LM--;
    LT += getdayspermonth(LM, Y);
  }

  return LT;
}  

int elapsed_sleep_days(time_t t, uint8_t channel) {
 /*
  * check if we are within sleep time and if yes, return the number of minutes
  * to add to *_delay
  */
 if(db.programs[db.channels[channel].program].sleep_day > 0 && db.programs[db.channels[channel].program].sleep_mon > 0) {
   // ok, sleep time enabled
   int cur_day = getdayofyear(day(t), month(t), year(t));
   int con_day = getdayofyear(db.programs[db.channels[channel].program].sleep_day, db.programs[db.channels[channel].program].sleep_mon, year(t));
   //Serial << channel << ": cur: " << cur_day << ", con: " << con_day << ", ";
   //Serial << db.programs[db.channels[channel].program].sleep_day << ", " << db.programs[db.channels[channel].program].sleep_mon << ", " << year(t) << endl;
   if(cur_day >= con_day) {
     // ok, sleep date is earlier than current date
     return (cur_day - con_day) * db.programs[db.channels[channel].program].sleep_increment;
   }
 }
 return 0;
}

void software_Reset() {
  /*
   * Restarts program from beginning but does not reset the peripherals and registers
   * via: http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1273155873/10#10
   */
  asm volatile ("  jmp 0");
} 

bool dst(time_t t) {
  /*
   * calculate daylight savings time.
   * returns true if given time_t is within sommertime.
   *
   * via: http://www.mikrocontroller.net/attachment/8391/TIME.C
   */
  int monat     = month(t);
  int tag       = day(t);
  int wochentag = weekday(t);
  int stunde    = hour(t);
  
  if( monat < 3 || monat > 10 ) {
    return false;
  }
  
  if( tag - wochentag >= 25 && (wochentag || stunde >= 2) ) {
    if( monat == 10 ) {
       return false;
     }
  }
  else {
    if( monat == 3 ) {
      return false;
    }
  }

  return true;
}

time_t gettimeofday() {
  /* return current time with dst() applied */
  t = now();
  if(dst(t)) {
    return t + 3600; 
  }
  else {
    return t;
  }
}

int getsunrise(time_t t) {
  int monat    = month(t);
  int tag      = day(t);
  int s = sunrise[monat - 1][tag - 1];
  if(dst(t)) {
    s += 60;
  }
  return s;
}

int getsunset(time_t t) {
  int monat    = month(t);
  int tag      = day(t);
  int s = sunrise[monat + 11][tag - 1];
  if(dst(t)) {
    s += 60;
  }
  return s;
}

char digits[3];
char* N(int number) {
   /* return a null if the given number is einstellig */
   sprintf(digits, "%02d", number);
   return digits;
}

int operate(int channel, time_t t) {
  /*
   * check if given channel has to be turned on
   * based on eeprom program. checks channel program
   * for sunrise or static timers.
   *
   * returns true if channel must be turned on,
   * false otherwise.
   */

  Program prog = db.programs[db.channels[channel].program];
      
  int h;
  int m;
  int cur;
  int begin;
  int end;

  if(prog.type == STATIC) {
    /* static (manual) start/stop timers */
    begin = (prog.start_hour * 60) + prog.start_min;
    end   = (prog.stop_hour * 60)  + prog.stop_min;
  }
  else {
    // fetch sunrise for current day from progmem table
    begin = getsunrise(t);
    delay(1); /* this delay is to fix a bug in flash library */
    end   = getsunset(t);

    begin += prog.start_delay + elapsed_sleep_days(t, channel);
    end   -= prog.stop_delay  + elapsed_sleep_days(t, channel);
  }

  // current time in minutes (seconds ignored)
  h = hour(t);
  m = minute(t);
  cur = (h * 60) + m;

  if(cur >= begin && cur <= end) {
    return HIGH;
  }
  else {
    return LOW;
  }
}

float temperature() {
  /* read sensor T */
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial << f_failed_dht << endl;
  }
  else {
    return t;
  }
}

float humidity() {
  /* read sensor H */
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial << f_failed_dht << endl;
  }
  else {
    return h;
  }
}

void blink() {
  /*
   * blinks the status led during boot.
   * It loops 3times, lights up for 25ms
   * and lights down for 100ms, so:
   * short on, long off, so that the
   * blinking is visible
   */
  for(i=0; i<3; i++) {
    digitalWrite(statusled, HIGH);
    delay(10);
    digitalWrite(statusled, LOW);
    delay(50);
  }
}

void beep() {
  /* play a C-Dur 4 for 300 milliseconds - a beep */
  playTone(NOTE_C, 300);
}


void alarm() {
  /* play an alarm melody */
  playTone(NOTE_B, 300);
  playTone(NOTE_E, 100);
  delay(100);
  playTone(NOTE_G, 500);
  delay(300);
  
  playTone(NOTE_B, 300);
  playTone(NOTE_E, 100);
  delay(100);
  playTone(NOTE_G, 500);
  delay(300);
  
  playTone(NOTE_B, 300);
  playTone(NOTE_E, 100);
  delay(100);
  playTone(NOTE_G, 500);
}

void check_main(bool init) {
  /*
   * Check if main switch were pressed and set manual
   * accordingly. If init is true, previous state
   * is ignored and mainstatus will be initialized.
   */
  pressed = digitalRead(mainswitch);
 
  if(pressed != mainstatus || init) {
    if(pressed) {
      manual = false;
      digitalWrite(mainled, HIGH);
    }
    else {
      manual = true;
      digitalWrite(mainled, LOW);
    }
    mainstatus = pressed;
    check_switches(true);
    check_timers(true);
    //Serial << "main - pressed: " << pressed << ", mainstatus: " << mainstatus << ", manual: " << manual << endl;
  }
}


void check_switches(bool init) {
  /*
   * Check if switch for the given channel id
   * were pressed. If current mode is manual,
   * switch relay+led accordingly, otherwise do nothing
   * with relay and keep led off.
   * If init is true, previous state
   * is ignored and state[channel] will be initialized.
   */ 

  // timers only used for cooldown feature,
  // because this func doesnt have its own timers
  stmoment = millis();
  if(stmoment < sttimer) {
    // millis() rolled over, 50 days are gone, reset timer
    sttimer = 0;
  }

  for(channel=0; channel<numswitches; channel++) {
    swpressed = digitalRead(switches[channel]);
    if(swpressed && (manual || db.programs[db.channels[channel].program].type == MANUAL)) {
      // remember how long the channel is already running
      if(db.programs[db.channels[channel].program].cooldown > 0) {
        if((cooldown[channel] / 1000) - 1 < (db.programs[db.channels[channel].program].cooldown * 60)) {
          cooldown[channel] += (stmoment - sttimer);
        }
        else {
          // reached cooldowntime, init artificially
          state[channel] = 0;
        }
      }
      if(cooldown[channel] < (db.programs[db.channels[channel].program].cooldown * 60)) {
         cooldown[channel] += (int)(stmoment - sttimer) / 1000;
       }
    }
    if(swpressed != state[channel] || init) {
      if(manual || db.programs[db.channels[channel].program].type == MANUAL) {
	if(swpressed) {
	  digitalWrite(leds[channel],   HIGH);
	  if(db.programs[db.channels[channel].program].cooldown) {
	    if((cooldown[channel] / 1000) > (db.programs[db.channels[channel].program].cooldown * 60)) {
	      // ok, it's cooled down enough, switch it
	      digitalWrite(relays[channel], LOW);
	    }
	  }
	  else {
	    digitalWrite(relays[channel], LOW);
	  }
	}
	else {
	  digitalWrite(leds[channel],   LOW);
	  digitalWrite(relays[channel], HIGH);
          cooldown[channel] = 0;
	}
      }
      else {
	digitalWrite(leds[channel],   LOW);
      }
      state[channel] = swpressed;
      //Serial << "checkswitches" << endl;
      //Serial << "   channel: " << channel << ", pressed: " << swpressed << ", state: " << state[channel];
      //Serial << ", stmoment: " << stmoment << ", sttimer: " << sttimer << ", man: " << manual << endl;
    }
  }

  sttimer = stmoment;
}



void check_timers(bool init) {
  /*
   * Schedules programchecks for all channels
   * runs only if elapsed time is > timerintervall
   * and if this is the case, only checks programs
   * if manual mode is currently off.
   * If manual mode is off, then it switches relays
   * if operate() tells it, that current time is
   * within operating range (runtime)
   */
  if(! manual) {
    moment = millis();
    if(moment < timer) {
      // millis() rolled over, 50 days are gone, reset timer
      timer = 0;
    }

    if(init || moment - timer > timerintervall) {
      t = gettimeofday();
      for(channel=0; channel<numchannels; channel++) {
        if(db.programs[db.channels[channel].program].type == MANUAL) {
          continue; 
        }
        
	runtime = operate(channel, t);
        
        if(db.programs[db.channels[channel].program].cooldown > 0) {
          // program has cooldown time configured
          if(runtime) {
            // proposed runtime
            if((cooldown[channel] / 1000) - 1 > (db.programs[db.channels[channel].program].cooldown * 60)) {
              // was long enohg off, don't wait anymore, turn it on
              runstate[channel] = 0;
            }
            else {
              // wait more, time not elapsed yet
              cooldown[channel] += (moment - timer);
            }
          }
          else {
            // shall be off
            if((cooldown[channel] / 1000) - 1 < (db.programs[db.channels[channel].program].cooldown * 60)) {
              // still time left, add to cooldown more
              cooldown[channel] += (moment - timer);
            }
          }
        }

	if(runtime != runstate[channel] || init) {
	  if(runtime) {
	    /* within operation time, turn the channel on */
	    if(db.programs[db.channels[channel].program].cooldown > 0) {
	      if((cooldown[channel] / 1000) > (db.programs[db.channels[channel].program].cooldown * 60)) {
		// ok, it's cooled down enough, switch it
		digitalWrite(relays[channel], LOW);
	      }
	    }
	    else {
	      digitalWrite(relays[channel], LOW);
	    }
	  }
	  else {
	    digitalWrite(relays[channel], HIGH);
            cooldown[channel] = 0;
	  }
	  runstate[channel] = runtime;
	}
      }
      timer = moment;
    
      // log uptime
      if(uptime < MAXUP) {
        uptime++;
      }
    }
  }
}

void sh_temphumidate() {
  /*
   * show temperature, humidity, date/time, sunrise and sunset
   */
  float T  = temperature();
  float h  = humidity();
  t        = gettimeofday();
  int begin = getsunrise(t);
  int end   = getsunset(t);
  int sunrisehour   = (begin - (begin % 60)) / 60;
  int sunriseminute = begin % 60;
  int sunsethour    = (end - (end % 60)) / 60;
  int sunsetminute  = end % 60;

  Serial << f_sht_temp << T << f_grad << endl;
  Serial << f_sht_hum << h << f_percent << endl;
  Serial << N(hour(t)) << ':' << N(minute(t)) << ':' << N(second(t)) << ' ' << N(day(t)) << '.' << N(month(t)) << '.' << year(t) << endl;

  Serial << f_sht_aircond;
  if(db.settings.aircondition) {
    if(airon) {
      Serial << f_sht_run<< endl;
    }
    else {
      Serial << f_sht_run << endl;
    }
    Serial << f_sht_tmin << db.settings.air_tmin << f_grad << endl;
    Serial << f_sht_tmax << db.settings.air_tmax << f_grad << endl;
    Serial << endl;
  }
  else {
    Serial << f_sht_off << endl;
  }

  if(dst(t)) {
    Serial << f_sht_som << endl;
  }
  else {
    Serial << f_sht_win << endl;
  }

  Serial << f_sht_sunr << N(sunrisehour) << ':' << N(sunriseminute) << endl;
  Serial << f_sht_suns << N(sunsethour)  << ':' << N(sunsetminute)  << endl;
}

void sh_channels() {
  /*
   * shows all channel configs with program assignments
   */
  for(channel=0; channel<numchannels; channel++) {
    Serial << f_shc_cn << channel << f_colon << names[channel] << f_shc_cfg << endl;
    Serial << f_shc_pr << db.channels[channel].program << endl;
    Serial << f_shc_typ;
    if(db.programs[db.channels[channel].program].type == STATIC) {
      Serial << f_shc_st << endl;
      Serial << f_shc_start << N(db.programs[db.channels[channel].program].start_hour) << ':' << N(db.programs[db.channels[channel].program].start_min) << endl;
      Serial << f_shc_stop  << N(db.programs[db.channels[channel].program].stop_hour)  << ':' << N(db.programs[db.channels[channel].program].stop_min)  << endl;
    }
    else if(db.programs[db.channels[channel].program].type == MANUAL) {
      Serial << f_shc_man << endl;
    }
    else {
      Serial << f_shc_ast << endl;
      Serial << f_shc_stdel  << db.programs[db.channels[channel].program].start_delay << f_shc_min << endl;
      Serial << f_shc_stodel << db.programs[db.channels[channel].program].stop_delay  << f_shc_min << endl;
    }
    if(db.programs[db.channels[channel].program].cooldown > 0) {
      Serial << f_shc_delay << db.programs[db.channels[channel].program].cooldown << f_shc_min << endl;
    }
  }
}

void sh_programs() {
  /*
   * show all saved programs in EEPROM
   */
  for(program = 0; program<maxprograms; program++) {
    Serial << f_shc_pn << program << endl << f_shc_typ;
    if(db.programs[program].type == STATIC) {
       Serial << f_shc_st << endl;
       Serial << f_shc_start << N(db.programs[program].start_hour) << ':' << N(db.programs[program].start_min) << endl;
       Serial << f_shc_stop  << N(db.programs[program].stop_hour)  << ':' << N(db.programs[program].stop_min)  << endl;
    }
    else if(db.programs[program].type == MANUAL) {
      Serial << f_shc_man << endl;
    }
    else {
      Serial << f_shc_ast << endl;
      Serial << f_shc_stdel  << db.programs[program].start_delay << f_shc_min << endl;
      Serial << f_shc_stodel << db.programs[program].stop_delay  << f_shc_min << endl;
    }
    if(db.programs[program].cooldown > 0) {
      Serial << f_shc_delay << db.programs[program].cooldown << f_shc_sec << endl;
    }
  }
}

void sh_air(char parameter[MAXBYTES]) {
  /*
   * sets the air condition based on user serial input
   */
  if(parameter[0] != '\0') {
    // air params given, parse it
    int value[2];
    int nvalue;
    char buffer[2];
    byte idx = 0;
    for(i=0; parameter[i]; i++) {
      if(parameter[i] > '0' || parameter[i] < '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == ':') {
	// one value done
	value[nvalue] = atoi(buffer);
	nvalue++;
	idx = 0;
	buffer[0] = '\0';
      }
      else {
	// no digit, no dot = fail!
	beep();
	Serial << f_sherr_char << parameter[i] << '!' << endl;
	return;
      }
    }
    if(nvalue != 1) {
      beep();
      Serial << f_sherr_form <<  f_sherr_extmp << endl;
      return;
    }
    else {
      if(value[0] < 0 || value[0] > 65) {
	beep();
        Serial << f_sherr_form << f_sherr_exstmp << endl;
	return;
      }
      if(value[1] < 0 || value[1] > 65) {
	beep();
        Serial << f_sherr_form << f_sherr_exstmp << endl;
	return;
      }

      // if we have done it until here, change the date
      Settings S     = db.settings;
      S.aircondition = true;
      S.air_tmin     = value[0];
      S.air_tmax     = value[1];
      ee_wr_settings(S);
      db = ee_getdb();
      Serial << f_sht_tmpsav << value[0] << '-' << value[1] << endl;
      return;
    }
  }
  else {
    // no params given, turn it off
    Settings S     = db.settings;
    S.aircondition = false;
    ee_wr_settings(S);
    db = ee_getdb();

    Serial << f_sht_airoff << endl;
    return;
  }
}


void sh_ip(char parameter[MAXBYTES]) {
  /*
   * sets the ip address based on user serial input
   */
  if(parameter[0] != '\0') {
    // ip given, parse it
    nvalue = 0;
    idx    = 0;
    for(i=0; parameter[i]; i++) {
      if(parameter[i] >= '0' && parameter[i] <= '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == '.') {
	// one octet done
	value[nvalue] = atoi(buffer);
	nvalue++;
	idx = 0;
	buffer[0] = '\0';
      }
      else {
	// no digit, no dot = fail!
	beep();
	Serial << f_sherr_char << parameter[i] << '!' << endl;
	return;
      }
    }
    if(buffer[0] != '\0') {
      value[nvalue] = atoi(buffer);
      nvalue++;
      idx = 0;
      buffer[0] = '\0';
    }
    
    if(nvalue != 4) {
      beep();
      Serial << f_sherr_form << f_sherr_exip << endl;
      return;
    }
    else {
      for(i=0; i<4; i++) {
	if(value[i] < 0 || value[i] > 255) {
	  beep();
          Serial << f_sherr_form << f_sherr_exoct << endl;
	  return;
	}
      }
      // if we have done it until here, put the octets into eeprom
      Settings S = db.settings;
      S.octet1 = value[0];
      S.octet2 = value[1];
      S.octet3 = value[2];
      S.octet4 = value[3];
      ee_wr_settings(S);
      db = ee_getdb();
      uint8_t ip[] = { db.settings.octet1, db.settings.octet2, db.settings.octet3, db.settings.octet4 };
      Ethernet.begin(mac, ip);
      Serial << f_sht_ipsav << value[0] << '.' << value[1] << '.' << value[2] << '.' << value[3] << endl;
      
      return;
    }
  }
  else {
    Serial << f_ship << db.settings.octet1 << '.' << db.settings.octet2 << '.';
    Serial << db.settings.octet3 << '.' << db.settings.octet4 << endl;
    Serial << ipgw << db.settings.gw1 << '.' << db.settings.gw2 << '.';
    Serial << db.settings.gw3 << '.' << db.settings.gw4 << endl;
  }
}

void sh_setdate(char parameter[MAXBYTES]) {
  /*
   * sets the date based on user serial input
   */
  if(parameter[0] != '\0') {
    // date given, parse it
    nvalue = 0;
    idx = 0;
    for(i=0; parameter[i]; i++) {
      if(parameter[i] >= '0' && parameter[i] <= '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == '.') {
	// one value done
	value[nvalue] = atoi(buffer);
	nvalue++;
	idx = 0;
	buffer[0] = '\0';
      }
      else {
	// no digit, no dot = fail!
	beep();
	Serial << f_sherr_char << parameter[i] << endl;
	return;
      }
     }
    
   if(buffer[0] != '\0') {
        value[nvalue] = atoi(buffer);
        nvalue++;
        idx = 0;
        buffer[0] = '\0';
    }

    if(nvalue != 3) {
      beep();
      Serial << f_sherr_form << f_sherr_ex3digdot << endl;
      return;
    }
    else {
      if(value[0] < 1 || value[0] > 31) {
	beep();
        Serial << f_sherr_form << f_sherr_exday << endl;
	return;
      }
      if(value[1] < 1 || value[1] > 12) {
	beep();
        Serial << f_sherr_form << f_sherr_exmon << endl;
	return;
      }
      if(value[2] < 2010 || value[2] > 3600) {
	beep();
        Serial << f_sherr_form << f_sherr_exyea << endl;
	return;
      }

      // if we have done it until here, change the date
      t = now();
      setTime(hour(t), minute(t), second(t), value[0], value[1], value[2]);
      t = now();
      RTC.set(t);

      Serial << f_sht_datesav << N(value[0]) << '.' << N(value[1]) << '.' << value[2] << endl;
      return;
    }
  }
  else {
    beep();
    Serial << f_sherr_ddmmmis << endl;
  }
}

void sh_settime(char parameter[MAXBYTES]) {
  /*
   * sets the time based on user serial input
   */
  if(parameter[0] != '\0') {
    // time given, parse it
    for(i=0; parameter[i]; i++) {
      if(parameter[i] >= '0' && parameter[i] <= '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == ':') {
	// one value done
	value[nvalue] = atoi(buffer);
	nvalue++;
	idx = 0;
	buffer[0] = '\0';
      }
      else {
	// no digit, no dot = fail!
	beep();
	Serial << f_sherr_char << parameter[i] << endl;
	return;
      }
    }
    
    if(buffer[0] != '\0') {
       value[nvalue] = atoi(buffer);
	nvalue++;
	idx = 0;
	buffer[0] = '\0';
    }
      
    if(nvalue != 3) {
      beep();
      Serial << f_sherr_form << f_sherr_ex3dig <<  nvalue << endl;
      return;
    }
    else {
      if(value[0] < 0 || value[0] > 23) {
	beep();
        Serial << f_sherr_form << f_sherr_exhour << endl;
	return;
      }
      if(value[1] < 0 || value[1] > 59) {
	beep();
        Serial << f_sherr_form << f_sherr_exmin << endl;
	return;
      }
      if(value[2] < 0 || value[2] > 59) {
	beep();
        Serial << f_sherr_form << f_sherr_exsec << endl;
	return;
      }

      // if we have done it until here, change the date
      t  = gettimeofday();
      int tag    = day(t);
      int monat  = month(t);
      int jahr   = year(t);
      setTime(value[0], value[1], value[2], tag, monat, jahr);
      t = now();
      RTC.set(t);
      
      Serial << f_sht_timesav<< N(value[0]) << ':' << N(value[1]) << ':' << N(value[2]) << endl;
      return;
    }
  }
  else {
    beep();
    Serial << f_sherr_hhmmmis << endl;
  }
}


char displaypin(int pin) {
  if(pin) {
    return PINON;
  }
  else {
    return PINOFF;
  } 
}

void sh_pins() {
  /*
   * Display a table of our PINs with their current state
   */
  t = gettimeofday();
  Serial << f_shp_tit << endl << endl;
  Serial << f_shp_head << endl;
  Serial << f_shp_line << endl;
  Serial << f_shp_pin << mainswitch << f_shp_2sp;
  for(channel=0; channel<numswitches; channel++) {
    Serial << f_shp_pipe << switches[channel] << f_shp_2sp;
  }
  Serial << endl;
  Serial << f_shp_line << endl;
  
  Serial << f_shp_mode << displaypin(digitalRead(mainswitch)) << f_shp_2sp;
  for(channel=0; channel<numchannels; channel++) {
    Serial << f_shp_pipe1 << displaypin(state[channel]) << f_shp_2sp;
  }
  Serial << endl;
  Serial << f_shp_line << endl;
  Serial << f_shp_rel;
  for(channel=0; channel<numchannels; channel++) {
    int ops = operate(channel, t);
    Serial << f_shp_pipe1;
    if(manual) {
      Serial << displaypin(state[channel]); 
    }
    else {
      Serial << displaypin(ops);
    }
    Serial << f_shp_2sp;
  }
  Serial << endl << endl;
}

void check_command(int command, char parameter[MAXBYTES]) {
  /*
   * Execute command taken from serial port
   */
  if (command == 'r') {
    Serial << f_sh_reset << endl;
    ee_init(INIT);
    Serial << f_sh_initres << endl;
    software_Reset();
  }
  else if(command == 'b') {
    Serial << f_sh_initres << endl;
    software_Reset();
  }
  else if(command == 's') {
    sh_temphumidate();
  }
  else if(command == 'c') {
    sh_channels();
  }
  else if(command == 'p') {
    sh_programs();
  }
  else if(command == 'i') {
    sh_ip(parameter);
  }
  else if(command == 'd') {
    sh_setdate(parameter);
  }
  else if(command == 't') {
    sh_settime(parameter);
  }
  else if(command == 'P') {
    sh_pins();
  }
  else if(command == 'a') {
    sh_air(parameter);
  }
  else if(command == 'm') {
    Serial << f_mem <<  freeMemory() << endl;
  }
  else if(command == 'h' || command == '?') {
    Serial << f_sh_help;
  }
  else {
    beep();
    Serial << f_sh_inv << command << endl;
  }
}




void check_shell() {
  /*
   * Simulate a simple shell on serial port, so we can
   * get some infos from the controller if anything else
   * fails
   */
   
  while (Serial.available() > 0) {
    onebyte = Serial.read();
    if(onebyte == '\r' ||onebyte == '\n') {
      // command sequence complete
      check_command(command, parameter);
      index        = 0;
      parameter[0] = '\0';
      command      = '\0';
      parametermode= false;
      Serial << f_prompt;
    }
    else if(onebyte == ' ') {
      parametermode = true;
    }
    else {
      if(parametermode) {
	if(index == MAXBYTES) {
	  beep();
	  index        = 0;
	  parameter[0] = '\0';
	  command      = '\0';
          parametermode= false;
	}
	else {
	  parameter[index] = onebyte;
	  index++;
	  parameter[index] = '\0';
	}
      }
      else {
	command = onebyte;
      }
    }
  }
}


void check_air(bool init) {
  /*
   * Turns on aircondition if Tmax has been reached
   * and turns it off if Tmin has been reached.
   */
    airmoment = millis();
    if(airmoment < airtimer) {
      // millis() rolled over, 50 days are gone, reset timer
      airtimer = 0;
    }

    if(init || airmoment - airtimer > airtimerinterval) {
      float T = temperature();

      //Serial << "alarmpaused: " << alarmpaused << ", T: " << T << ", min: " << db.settings.air_alarm << ", wait: " << alarmwait;
      //Serial << ", elapsed: " << airmoment - airtimer << ", interval: " << airtimerinterval << endl;

      if(db.settings.aircondition) {
        if(T > db.settings.air_tmax) {
	  // turn air condition on
	  digitalWrite(air, LOW);
	  airon = true;
        }
        else if(T < db.settings.air_tmin) {
	  // turn air condition off
	  digitalWrite(air, HIGH);
	  airon = false;
        }
      }

      if(db.settings.air_alarm > 0) {
        // check air alarm
        t = gettimeofday();
        
        if(operate(0, t) ||Â operate(1, t)) {
          // ok, one of the 70W channels is running
          if((alarmpaused / 60000) > alarmwait) {
            // ok, the alarm paused long enough
            if(T < db.settings.air_alarm) {
              // ok, it's still too cold
              Serial << f_sherr << endl;
              alarm();
              alarmpaused = 0;
            }
          }
          else {
            // not enough tim elapsed to raise an alarm
            alarmpaused += ( airmoment - airtimer );
          }
        }
        else {
          // out of operating time
          alarmpaused = 0; 
        }
      }
      airtimer = airmoment;
    }
}



void setup() {
  beep();
  pinMode(statusled,  OUTPUT);

  blink();
  Serial.begin(9600);

  blink();
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial << f_rtc_fail << endl;
  else
     Serial << init << f_rtc_ok << endl;

  t = now();
  
  blink();
  Serial << init << init_dht << endl;
  dht.begin();

  blink();
  Serial << init << init_wire << endl;
  Wire.begin();

  blink();
  Serial << init << init_eep << endl;
  ee_init(RUN);
  
  blink();
  Serial << init_dbread << endl;
  db = ee_getdb();

  if(dbversion > db.header.version) {
    alarm();
    Serial << init_dberror << endl;
    ee_init(INIT);
    db = ee_getdb();    
  }
  else {
    Serial << init_dbok db.header.version << endl;
  }
  
  blink();
  Serial << init << init_speak << endl;
  pinMode(speaker,    OUTPUT); 

  blink();
  Serial << init << init_air << endl;
  pinMode(air,        OUTPUT);
  digitalWrite(air, HIGH);
  delay(5);
  check_air(INIT);
  
  blink();
  Serial << init << init_sw << endl;
  pinMode(mainswitch, INPUT);
  pinMode(mainled,    OUTPUT);

  check_main(INIT);
  
  for(channel=0; channel<numswitches; channel++) {
    blink();
    Serial << channel;
    pinMode(switches[channel], INPUT);
    pinMode(leds[channel],     OUTPUT);
  }
 
  blink();
  Serial << init << init_relay; 
  for(channel=0; channel<numchannels; channel++) {
    pinMode(relays[channel],   OUTPUT);
    Serial << channel << ' ';
  }
  Serial << init_ok << endl;

  blink();
  Serial << init << init_timers << endl;
  check_timers(INIT);

  blink();
  Serial << init << init_eth << db.settings.octet1 << '.' << db.settings.octet2 << '.' << db.settings.octet3 << '.' << db.settings.octet4 << endl;
  uint8_t ip[] = { db.settings.octet1, db.settings.octet2, db.settings.octet3, db.settings.octet4 };
  uint8_t gw[] = { db.settings.gw1, db.settings.gw2, db.settings.gw3, db.settings.gw4 };
  Ethernet.begin(mac, ip, gw);

  blink();
  Serial << init << init_www << endl;
  webserver.setDefaultCommand(&www_home);
  webserver.setFailureCommand(&www_home);
  webserver.addCommand("channels.html",   &www_channels);
  webserver.addCommand("setdate.html",    &www_setdate);
  webserver.addCommand("setip.html",      &www_setip);
  webserver.addCommand("setprogram.html", &www_setprogram);
  webserver.addCommand("programs.html",   &www_programs);
  webserver.addCommand("editprogram.html",&www_editprogram);
  webserver.addCommand("air.html",        &www_air);
  webserver.addCommand("rrd.html",        &www_rrd);
  webserver.begin();

  
  /* finally enable watchdog and set PAT timeout */
  blink();
  Serial << init << init_wdt << WDTO_4S << 's' << endl;
  wdt_enable(WDTO_4S);

  /* booting done, keep status on */
  digitalWrite(statusled, HIGH);
  beep();
  Serial << f_mem <<  freeMemory() << endl << init_done << endl << endl;
  Serial << f_prompt;
  
  booted = gettimeofday();
}


void loop() {
  check_main(RUN);
  check_switches(RUN);
  check_timers(RUN);
  check_air(RUN);
  check_shell();
  webserver.processConnection();
  wdt_reset();
}
