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

#include <stdio.h>

/* defines */
#define SUNRISE 0
#define STATIC  1
#define DHTPIN 25
#define DHTTYPE DHT22 
#define NOTE_C4  262
#define WEBDUINO_FAIL_MESSAGE "<h1>Request Failed</h1>"
#define PREFIX ""
#define MAXBYTES 64

/* global vars */
time_t t;
int startdelay;
int stopdelay;
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char endl = '\n';
bool manual = false;
uint8_t mainstatus = 0;
uint8_t pressed    = 0;
uint8_t swpressed  = 0;
long timerintervall = 500;
long timer;
long moment;
long sttimer;
long stmoment;
long airtimerinterval = 7200; // 10 min
long airtimer;
long airmoment;
bool airon;
int channel;
bool INIT = true;
bool RUN  = false;
uint8_t runtime;
long cooldown[] = {0, 0, 0, 0, 0, 0};

// serial parser vars
int     value[4];
uint8_t nvalue = 0;
char    buffer[5];
byte    idx = 0;
    
uint8_t onebyte;
uint8_t command;
char parameter[MAXBYTES];
byte index = 0;
bool parametermode = false;

/* important for EEany initialization */
int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1000;

/* post parser vars */
char parsename[32];
char parsevalue[32];
bool parsing = true;

/* PINs */
const uint8_t switches[] = { 15, 49, 47, 45, 43, 41 };
const uint8_t leds[]     = { 16, 48, 46, 44, 42, 40 };
const uint8_t relays[]   = { 36, 34, 32, 30, 28, 26 };
uint8_t state[]          =  { 0, 0, 0, 0, 0, 0 };
uint8_t runstate[]       =  { 0, 0, 0, 0, 0, 0 };
String names[] = { "70 W Links", "70 W Rechts", "40 W Vorn", "40 W Hinten", "15 W Links", "15 W Rechts"};
const uint8_t air        = 24;
const uint8_t statusled  = 27;
const uint8_t speaker    = 17;
const uint8_t mainswitch = 19;
const uint8_t mainled    = 18;


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
#define NOTE_A 1136
#define NOTE_B 1014
#define NOTE_C 1915
#define NOTE_D 1700
#define NOTE_E 1519
#define NOTE_F 1432
#define NOTE_G 1275

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
  
  if(db.programs[p].type == 1) {
    server << f_shc_st << f_shc_start;
    server<< db.programs[p].start_hour << ':' << db.programs[p].start_min;
    server << bis << f_shc_stop;
    server << db.programs[p].stop_hour << ':' << db.programs[p].stop_min;
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
  server << tra << tdr << itime << tde << td << post.i3 << '.' << post.i4 << '.' << post.i5;
  server << ' ' << post.i6 << ':' << post.i7 << ':' << post.i8 << tde << tre;
  
  server << tra << tdr << imode << tde << td ;
  if(manual) {
    server << imm;
  }
  else {
    server << ima;
  }
  server << tde << tre;

  server << tra << tdr << isunr << tde << td << post.i9 << ':' << post.i10 << tde << tre;
  server << tra << tdr << isuns << tde << td  << post.i11 << ':' << post.i12 << tde << tre;
  
  server << tra << tdrt << ichan << tde << td;
  for (uint8_t i=0; i<6; i++) {
    server << names[i] << ':';
    if(manual) {
      if(state[i] == HIGH) {
        server << irun;
      }
      else {
        server << ioff;
      }
    }
    else {
      if(operate(i, t)) {
        server << irun;
      }
      else {
        server << ioff;
      }
    }
    server << br;
  }
  server << tde << tre << tablee << hfoot;
}

void tpl_channels(WebServer &server) {
  server << hhead << tchannels << hmenu << chc;
  server << tablechan;

  for(int i=0; i<6; i++) {
    server << tra << td;
    server << chlinkl << i << chlinkr << names[i] << chlinke << tde;
    server << td;
    prog2web(db.channels[i].program, server);
    server << tde;
  }
  server << tre << tablee << hfoot;
}

void tpl_setdate(WebServer &server) {
  server << hhead << tdate << hmenu << chd;
  server << msg1 << post.s1 << msg2;
  server << sdform << tablesd;

  server << tra;

  server << sdfi2 << 0 << sdfv << post.i1 << sdfe << tde;
  server << sdfi2 << 1 << sdfv << post.i2 << sdfe << tde;
  server << sdfi4 << 2 << sdfv << post.i3 << sdfe << tde;
  server << sdfi2 << 3 << sdfv << post.i4 << sdfe << tde;
  server << sdfi2 << 4 << sdfv << post.i5 << sdfe << tde;
  server << sdfi2 << 5 << sdfv << post.i6 << sdfe << tde;

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
    for(int i=0; i<maxprograms; i++) {
      server << opt << i << '"';
      if(i == db.channels[post.i1].program) {
        server << selected;
      }
      server << '>';
      prog2web(i, server);
      server << opte;
    }
    server << sele << br << submit << forme;
  
  server << hfoot;
}

void tpl_programs(WebServer &server) {
  server << hhead << tprog << hmenu << chp;
  server << msg1 << post.s1 << msg2;
  server << tableprog;
  for(int i=0; i<maxprograms; i++) {
    server << tra << td;
    server << prlinkl << i << chlinkr << i << chlinke << td;
    prog2web(i, server);
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

  for (int i=0; i<2; i++) {
    server << opt << i << '"';
    if(db.programs[post.i1].type == i) {
      server << selected;
    }
    server << '>';
    if(i == STATIC) {
      server << f_shc_st;
    }
    else {
      server << f_shc_ast;
    }
    server << opte;
  }
  server << sele << tde << tre;

  // start 1:hour.i2 2:minute.i3
  server << tra << tdr << f_shc_start << tde << td;
  server << spf2 << 2 << sdfv << db.programs[post.i1].start_hour << sdfe << ':';
  server << spf2 << 3 << sdfv << db.programs[post.i1].start_min << sdfe;
  server << tde << tre;

  // stop 3:hour.i4 4:minute.i5
  server << tra << tdr << f_shc_stop << tde << td;
  server << spf2 << 4 << sdfv << db.programs[post.i1].stop_hour << sdfe << ':';
  server << spf2 << 5 << sdfv << db.programs[post.i1].stop_min << sdfe;
  server << tde << tre;

  // startdelay 5:startdelay.i6
  server << tra << tdr << f_shc_stdel << tde << td;
  server << spf5 << 6 << sdfv << db.programs[post.i1].start_delay << sdfe;
  server << tde << tre;

  // startdelay 6:startdelay.i7
  server << tra << tdr << f_shc_stodel << tde << td;
  server << spf5 << 7 << sdfv << db.programs[post.i1].stop_delay << sdfe;
  server << tde << tre;

  // cooldown 7:cooldown.i8
  server << tra << tdr << spcooldown << tde << td;
  server << spf5 << 8 << sdfv << db.programs[post.i1].cooldown << sdfe;
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
  server << br << submit << forme << hfoot;
}

void tpl_setair(WebServer &server) {
  server << hhead << tair << hmenu << chair;
  server << msg1 << post.s1 << msg2;
  server << airform << tableair;

  server << spsel;

  server << opt << 0;
  if(db.settings.aircondition == 0) {
    server << selected;
  }
  server << '>' << airinactive << opte;

  server << opt << 1;
  if(db.settings.aircondition == 1) {
    server << selected;
  }
  server << '>' << airactive << opte;
  server << tde << tre;

  server << tra << tdr << tmin << tde;
  server << td << spf2 << 2 << sdfv << db.settings.air_tmin << sdfe << tde << tre;

  server << tra << tdr << tmax << tde;
  server << td << spf2 << 3 << sdfv << db.settings.air_tmax << sdfe << tde << tre;

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
    }

    Settings S = db.settings;
    S.octet1 = post.i1;
    S.octet2 = post.i2;
    S.octet3 = post.i3;
    S.octet4 = post.i4;
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
      if(post.i1 < 0 || post.i1 > 5) {
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
      if (strcmp(parsename, "0") == 0){post.i1 = atoi(parsevalue);}
      if (strcmp(parsename, "1") == 0){post.i2 = atoi(parsevalue);}
      if (strcmp(parsename, "2") == 0){post.i3 = atoi(parsevalue);}
      if (strcmp(parsename, "3") == 0){post.i4 = atoi(parsevalue);}
      if (strcmp(parsename, "4") == 0){post.i5 = atoi(parsevalue);}
      if (strcmp(parsename, "5") == 0){post.i6 = atoi(parsevalue);}
      if (strcmp(parsename, "6") == 0){post.i7 = atoi(parsevalue);}
      if (strcmp(parsename, "7") == 0){post.i8 = atoi(parsevalue);}
      if (strcmp(parsename, "8") == 0){post.i9 = atoi(parsevalue);}
    }

    int error = 0;
    if(post.i2 < 0 || post.i2 > 1) {
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
    }

    if(post.i2  < 0 || post.i2  > 65 || post.i3 < 0 || post.i3 > 65) {
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
      ee_wr_settings(S);
      airdone.copy(post.s1);
      db = ee_getdb();
    }
  }
  tpl_setair(server);
}


/*
 * Helper functions
 */
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

    begin += prog.start_delay;
    end   -= prog.stop_delay;
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
  for(int i=0; i<3; i++) {
    digitalWrite(statusled, HIGH);
    delay(10);
    digitalWrite(statusled, LOW);
    delay(20);
  }
}

void beep() {
  /* play a C-Dur 4 for 300 milliseconds - a beep */
  playTone(NOTE_C, 300);
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

  for(channel=0; channel<6; channel++) {
    swpressed = digitalRead(switches[channel]);
    if(swpressed && manual) {
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
      if(manual) {
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
      for(channel=0; channel<6; channel++) {
	runtime = operate(channel, t);
        if(runtime) {
          // remember how long the channel is already running
          if(db.programs[db.channels[channel].program].cooldown > 0) {
            if((cooldown[channel] / 1000) - 1 < (db.programs[db.channels[channel].program].cooldown * 60)) {
              cooldown[channel] += (moment - timer);
            }
            else {
              // reached cooldowntime, init artificially
              runstate[channel] = 0;
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
  Serial << hour(t) << ':' << minute(t) << ':' << second(t) << ' ' << day(t) << '.' << month(t) << '.' << year(t) << endl;

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

  Serial << f_sht_sunr << sunrisehour << ':' << sunriseminute << endl;
  Serial << f_sht_suns << sunsethour << ':' << sunsetminute << endl;
}

void sh_channels() {
  /*
   * shows all channel configs with program assignments
   */
  for(int i=0; i<6; i++) {
    Serial << f_shc_cn << i << f_colon << names[i] << f_shc_cfg << endl;
    Serial << f_shc_pr << db.channels[i].program << endl;
    Serial << f_shc_typ;
    if(db.programs[db.channels[i].program].type == STATIC) {
      Serial << f_shc_st << endl;
      Serial << f_shc_start << db.programs[db.channels[i].program].start_hour << ':' << db.programs[db.channels[i].program].start_min << endl;
      Serial << f_shc_stop << db.programs[db.channels[i].program].stop_hour << ':' << db.programs[db.channels[i].program].stop_min << endl;
    }
    else {
      Serial << f_shc_ast << endl;
      Serial << f_shc_stdel << db.programs[db.channels[i].program].start_delay << f_shc_min << endl;
      Serial << f_shc_stodel << db.programs[db.channels[i].program].stop_delay << f_shc_min << endl;
    }
    if(db.programs[db.channels[i].program].cooldown > 0) {
      Serial << f_shc_delay << db.programs[db.channels[i].program].cooldown << f_shc_min << endl;
    }
  }
}

void sh_programs() {
  /*
   * show all saved programs in EEPROM
   */
  for(int p=0; p<maxprograms; p++) {
    Serial << f_shc_pn << p << endl << f_shc_typ;
    if(db.programs[p].type == STATIC) {
       Serial << f_shc_st << endl;
       Serial << f_shc_start << db.programs[p].start_hour << ':' << db.programs[p].start_min << endl;
       Serial << f_shc_stop << db.programs[p].stop_hour << ':' << db.programs[p].stop_min << endl;
    }
    else {
      Serial << f_shc_ast << endl;
      Serial << f_shc_stdel << db.programs[p].start_delay << f_shc_min << endl;
      Serial << f_shc_stodel << db.programs[p].stop_delay << f_shc_min << endl;
    }
    if(db.programs[p].cooldown > 0) {
      Serial << f_shc_delay << db.programs[p].cooldown << f_shc_sec << endl;
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
    for(int i=0; parameter[i]; i++) {
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
    for(int i=0; parameter[i]; i++) {
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
      for(int i=0; i<4; i++) {
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
      // FIXME: maybe doesn't work this way, eventually reboot instead, check this!
      uint8_t ip[] = { db.settings.octet1, db.settings.octet2, db.settings.octet3, db.settings.octet4 };
      Ethernet.begin(mac, ip);
      Serial << f_sht_ipsav << value[0] << '.' << value[1] << '.' << value[2] << '.' << value[3] << endl;
      
      return;
    }
  }
  else {
    Serial << f_ship << db.settings.octet1 << '.' << db.settings.octet2 << '.';
    Serial << db.settings.octet3 << '.' << db.settings.octet4 << endl;
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
    for(int i=0; parameter[i]; i++) {
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

      Serial << f_sht_datesav << value[0] << '.' << value[1] << '.' << value[2] << endl;
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

    
    for(int i=0; parameter[i]; i++) {
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
      
      Serial << f_sht_timesav<< value[0] << ':' << value[1] << ':' << value[2] << endl;
      return;
    }
  }
  else {
    beep();
    Serial << f_sherr_hhmmmis << endl;
  }
}

#define PINON  'X'
#define PINOFF '-'
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
  for (int i=0; i<6; i++) {
    Serial << f_shp_pipe << switches[i] << f_shp_2sp;
  }
  Serial << endl;
  Serial << f_shp_line << endl;
  Serial << f_shp_mode << displaypin(digitalRead(mainswitch)) << f_shp_2sp;
  for (int i=0; i<6; i++) {
    Serial << f_shp_pipe1 << displaypin(state[i]) << f_shp_2sp;
  }
  Serial << endl;
  Serial << f_shp_line << endl;
  Serial << f_shp_rel;
  for (int i=0; i<6; i++) {
    int ops = operate(i, t);
    Serial << f_shp_pipe1;
    if(manual) {
      Serial << displaypin(state[i]); 
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
  if(db.settings.aircondition) {
    airmoment = millis();
    if(airmoment < airtimer) {
      // millis() rolled over, 50 days are gone, reset timer
      airtimer = 0;
    }

    if(init || airmoment - airtimer > airtimerinterval) {
      int T = (int)temperature;
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
     Serial << f_rtc_ok << endl;

  t = now();
  
  blink();
  Serial << "init DHT22" << endl;
  dht.begin();

  blink();
  Serial << "init Wire" << endl;
  Wire.begin();

  blink();
  Serial << "init eeprom" << endl;
  ee_init(RUN);
  
  blink();
  Serial << "read db from eeprom";
  db = ee_getdb();
  Serial << "    got db version ";
  Serial << db.header.version << endl;

  blink();
  Serial << "init speaker and air" << endl;
  pinMode(speaker,    OUTPUT); 
  pinMode(air,        OUTPUT);
  digitalWrite(air, HIGH);
  delay(5);
  check_air(INIT);

  blink();
  Serial << "init main switch" << endl;
  pinMode(mainswitch, INPUT);
  pinMode(mainled,    OUTPUT);
  check_main(INIT);

  Serial << "init switches ";
  for(int i=0; i<6; i++) {
    blink();
    Serial << i;
    pinMode(switches[i], INPUT);
    pinMode(leds[i],     OUTPUT);
    pinMode(relays[i],   OUTPUT);
  }
  Serial << endl;
  
  blink();
  Serial << "init timers" << endl;
  check_timers(INIT);

  blink();
  uint8_t ip[] = { db.settings.octet1, db.settings.octet2, db.settings.octet3, db.settings.octet4 };
  Ethernet.begin(mac, ip);

  blink();
  webserver.setDefaultCommand(&www_home);
  webserver.setFailureCommand(&www_home);
  webserver.addCommand("channels.html",   &www_channels);
  webserver.addCommand("setdate.html",    &www_setdate);
  webserver.addCommand("setip.html",      &www_setip);
  webserver.addCommand("setprogram.html", &www_setprogram);
  webserver.addCommand("programs.html",   &www_programs);
  webserver.addCommand("editprogram.html",&www_editprogram);
  webserver.addCommand("air.html",        &www_air);
  webserver.begin();


  /* booting done, keep status on */
  digitalWrite(statusled, HIGH);
  beep();
  //playNote(notes[0], beats[0] * tempo);
  Serial << f_mem <<  freeMemory() << endl;
  Serial << f_prompt;
}


void loop() {
  check_main(RUN);
  check_switches(RUN);
  check_timers(RUN);
  check_air(RUN);
  check_shell();
  webserver.processConnection();
}
