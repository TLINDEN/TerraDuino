/* -*-c-*-
 * Terraduino Terrarium Controller with Arduino.
 *
 * Copyright (c) 2012 Thomas Linden
 *
 */


/* Sunrise times stored as progmem table */
#include "Flash.h"
#include "Sunrise.h"

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
#include "SPI.h" // new include
#include "avr/pgmspace.h" // new include
#include "Ethernet.h"
#include "WebServer.h"

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
int mainstatus = 0;
int pressed    = 0;
long timerintervall = 500;
long timer;
long moment;
long airtimerinterval = 7200; // 10 min
long airtimer;
long airmoment;
bool airon;
int channel;
bool INIT = true;
bool RUN  = false;
int runtime;

int onebyte;
int command;
char parameter[MAXBYTES];
byte index = 0;
bool parametermode = false;

/* important for EEany initialization */
int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1000;

/* PINs */
int switches[] = { 51, 49, 47, 45, 43, 41 };
int leds[]     = { 50, 48, 46, 44, 42, 40 };
int relays[]   = { 36, 34, 32, 30, 28, 26 };
int state[]   =  { 0, 0, 0, 0, 0, 0 };
String names[] = { "70 W Links", "70 W Rechts", "40 W Vorn", "40 W Hinten", "15 W Links", "15 W Rechts"};
int air        = 24;
int statusled  = 27;
int speaker    = 23;
int mainswitch = 19;
int mainled    = 18;


/* global objects */
DHT dht(DHTPIN, DHTTYPE);
Database db;
WebServer webserver(PREFIX, 80);


/* Webserver template vars */
struct DATA_ChannelList {
  int id;
  int programid;
  String modus;
  String name;
  int operate;
  String program;
};

struct DATA_index {
  DATA_ChannelList Channels[6];
  int day;
  int hour;
  float humidity;
  int minute;
  String modus;
  int month;
  int second;
  int sunrisehour;
  int sunriseminute;
  int sunsethour;
  int sunsetminute;
  float temp;
  int year;
};

struct DATA_channels {
  DATA_ChannelList Channels[6];
};

struct DATA_setdate {
  int day;
  int dst;
  int hour;
  String message;
  int minute;
  int month;
  int second;
  int year;
};

struct DATA_setip {
  String message;
  uint8_t octet1;
  uint8_t octet2;
  uint8_t octet3;
  uint8_t octet4;
};

struct DATA_ProgramList {
  String current;
  String description;
  String modus;
  int id;
};

struct DATA_setprogram {
  DATA_ProgramList Programs[32];
  int id;
  String message;
};

struct DATA_programs {
  DATA_ProgramList Programs[32];
};

struct DATA_TypeList {
  String current;
  String description;
  int value;
};

struct DATA_editprogram {
  DATA_TypeList Types[2];
  int id;
  String message;
  int startdelay;
  int starthour;
  int startmin;
  int stopdelay;
  int stophour;
  int stopmin;
};

struct DATA_AirList {
  String current;
  String description;
  int id;
};

struct DATA_air {
  DATA_AirList Air[2];
  String message;
  int tmax;
  int tmin;
};


/* templates need to be included after template vars definitions */
#include "Templates.h"

/*
 * Webserver display functions
 */


void www_home(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }

  server.httpSuccess();

  if (type == WebServer::GET) {
    t  = gettimeofday();
    int begin = getsunrise(t);
    int end   = getsunset(t);

    DATA_index index_vars;
    for(int i=0; i<6; i++) {
      DATA_ChannelList c;
      c.name    = names[i];
      c.operate = operate(i, t);
      index_vars.Channels[i] = c;
    }

    if(manual) {
      index_vars.modus = "Manuell";
    }
    else {
      index_vars.modus = "Automatisch";
    }
    
    index_vars.hour   = hour(t);
    index_vars.minute = minute(t);
    index_vars.second = second(t);

    index_vars.day   = day(t);
    index_vars.month = month(t);
    index_vars.year  = year(t);

    index_vars.sunrisehour   = (begin - (begin % 60)) / 60;
    index_vars.sunriseminute = begin % 60;
    index_vars.sunsethour    = (end - (end % 60)) / 60;
    index_vars.sunsetminute  = end % 60;

    index_vars.temp     = temperature();
    index_vars.humidity = humidity();
 
    tpl_index(server, index_vars);
  }
}

void www_channels(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }

  server.httpSuccess();

  if (type == WebServer::GET) {
    DATA_channels data;

    for(int i=0; i<6; i++) {
      DATA_ChannelList c;
      c.name           = names[i];
      c.id             = i;
      c.programid      = db.channels[i].program;
      if(db.programs[db.channels[i].program].type == STATIC) {
	c.modus = "Statisch";
      }
      else {
	c.modus = "Astronomisch";
      }
      data.Channels[i] = c;
    }

    tpl_channels(server, data);
  }
}

void www_setdate(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  t  = gettimeofday();
  DATA_setdate data;

  server.httpSuccess();

  if (type == WebServer::POST)  {
    char name[32];
    char value[32];
    bool parsing = true;

    // initialize them, so that setTime() doesnt get undef's
    // in case the user didn't post them all
    int tag    = day(t);
    int monat  = month(t);
    int jahr   = year(t);
    int stunde = hour(t);
    int min    = minute(t);
    int sekunde= second(t);
    int dstz;

    if(dst(t)) {
      dstz = 1;
    }
    else {
      dstz = 0;
    }

    while (parsing) {
      parsing = server.readPOSTparam(name, 32, value, 32);
      if (strcmp(name, "day")    == 0){tag     = atoi(value);}
      if (strcmp(name, "month")  == 0){monat   = atoi(value);}
      if (strcmp(name, "year")   == 0){jahr    = atoi(value);}
      if (strcmp(name, "hour")   == 0){stunde  = atoi(value);}
      if (strcmp(name, "minute") == 0){min     = atoi(value);}
      if (strcmp(name, "second") == 0){sekunde = atoi(value);}
      if (strcmp(name, "dst")    == 0){dstz    = atoi(value);}
    }

    // set the time, first run, using raw input
    setTime(stunde, min, sekunde, tag, monat, jahr);

    if(dstz == 1) {
      // user supplied time is within dst, so substract 60 minutes
      // and set the corrected time again, since we run internally
      // ALWAYS within winter time
      t = gettimeofday();
      t =- 3600;
      adjustTime(t);
    }

    data.message = "Datum und Uhrzeit eingestellt";
  }

  t  = gettimeofday();
  data.hour   = hour(t);
  data.minute = minute(t);
  data.second = second(t);

  data.day   = day(t);
  data.month = month(t);
  data.year  = year(t);

  if(dst(t)) {
    data.dst = 1;
  }
  else {
    data.dst = 0;
  }

  tpl_setdate(server, data);
}


void www_setip(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  DATA_setip data;

  data.octet1 = db.settings.octet1;
  data.octet2 = db.settings.octet2;
  data.octet3 = db.settings.octet3;
  data.octet4 = db.settings.octet4;

  server.httpSuccess();

  if (type == WebServer::POST)  {
    char name[32];
    char value[32];
    bool parsing = true;

    while (parsing) {
      parsing = server.readPOSTparam(name, 32, value, 32);
      if (strcmp(name, "octet1") == 0){data.octet1 = atoi(value);}
      if (strcmp(name, "octet2") == 0){data.octet2 = atoi(value);}
      if (strcmp(name, "octet3") == 0){data.octet3 = atoi(value);}
      if (strcmp(name, "octet4") == 0){data.octet4 = atoi(value);}
    }

    db.settings.octet1 = data.octet1;
    db.settings.octet2 = data.octet2;
    db.settings.octet3 = data.octet3;
    db.settings.octet4 = data.octet4;

    ee_wr_settings(db.settings);

    data.message = "Neue IP Adresse eingestellt";
    db = ee_getdb();
  }

  tpl_setip(server, data);
}


void www_setprogram(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  DATA_setprogram data;
  DATA_ProgramList P;
  int channel;
  int program;

  server.httpSuccess();

  if (type == WebServer::GET)  {
    URLPARAM_RESULT rc;
    char name[32];
    char value[32];
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
	rc = server.nextURLparam(&url_tail, name, 32, value, 32);
	if (strcmp(name, "channel") == 0){channel = atoi(value);}
      }
      if(channel < 0 || channel > 5) {
	data.message = "Fehler: Unbekannter Steuerkanal!";
	tpl_setprogram(server, data);
	return;
      }
    }
    else {
      data.message = "Fehler: Channel ID nicht angegeben!";
      tpl_setprogram(server, data);
      return;
    }
  }

  if (type == WebServer::POST)  {
    char name[32];
    char value[32];
    bool parsing = true;

    while (parsing) {
      parsing = server.readPOSTparam(name, 32, value, 32);
      if (strcmp(name, "id")      == 0){channel = atoi(value);}
      if (strcmp(name, "program") == 0){program = atoi(value);}
    }

    if(program < 0 || program > 31) {
      data.message = "Fehler: Unbekanntes Programm!";
      tpl_setprogram(server, data);
      return;
    }
    else {
      db.channels[channel].program = program;
      ee_wr_channel(db.channels[channel]);
      data.message = "Steuerkanal neuem Programm zugeordnet";
      db = ee_getdb();
    }
  }

  for (int p=0; p<maxprograms; p++) {
    P = prog2web(p);
    if(db.channels[channel].program == p) {
      P.current = "selected='selected'";
    }
    data.Programs[p] = P;
  }

  tpl_setprogram(server, data);
}


void www_programs(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  DATA_programs data;
  DATA_ProgramList P;

  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }

  server.httpSuccess();

  if (type == WebServer::GET)  {
    for (int p=0; p<maxprograms; p++) {
      data.Programs[p] = prog2web(p);
    }
    tpl_programs(server, data);
  }
}


void www_editprogram(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  DATA_editprogram data;
  int program;
  int bh;
  int bm;
  int eh;
  int em;
  int bd;
  int ed;
  int typ;

  server.httpSuccess();

  if (type == WebServer::GET)  {
    URLPARAM_RESULT rc;
    char name[32];
    char value[32];
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
	rc = server.nextURLparam(&url_tail, name, 32, value, 32);
	if (strcmp(name, "id") == 0){program = atoi(value);}
      }
      if(program < 0 || program > 31) {
	data.message = "Fehler: Unbekanntes Programm!";
	tpl_editprogram(server, data);
	return;
      }
    }
    else {
      data.message = "Fehler: Program ID nicht angegeben!";
      tpl_editprogram(server, data);
      return;
    }
    bh   = db.programs[program].start_hour;
    bm   = db.programs[program].start_min;
    eh   = db.programs[program].stop_hour;
    em   = db.programs[program].stop_min;
    bd   = db.programs[program].start_delay;
    ed   = db.programs[program].stop_delay;
    typ  = db.programs[program].type;
  }

  if (type == WebServer::POST)  {
    char name[32];
    char value[32];
    bool parsing = true;

    while (parsing) {
      parsing = server.readPOSTparam(name, 32, value, 32);
      if (strcmp(name, "id")        == 0){program = atoi(value);}
      if (strcmp(name, "starthour") == 0){bh      = atoi(value);}
      if (strcmp(name, "startmin")  == 0){bm      = atoi(value);}
      if (strcmp(name, "stophour")  == 0){eh      = atoi(value);}
      if (strcmp(name, "stopmin")   == 0){em      = atoi(value);}
      if (strcmp(name, "startdelay")== 0){bd      = atoi(value);}
      if (strcmp(name, "stopdelay") == 0){ed      = atoi(value);}
      if (strcmp(name, "type")      == 0){typ     = atoi(value);}
    }

    int error;
    if(typ < 0 || typ > 1) {
      data.message = "Fehler: type != 0|1!";
      error = 1;
    }
    if(bh < 0 || bh > 23) {
      data.message = "Fehler: start hour ! 0-23!";
      error = 1;
    }
    if(bm < 0 || bm > 59) {
      data.message = "Fehler: start min ! 0-59!";
      error = 1;
    }
    if(eh < 0 || eh > 23) {
      data.message = "Fehler: stop hour ! 0-23!";
      error = 1;
    }
    if(em < 0 || em > 59) {
      data.message = "Fehler: stop min ! 0-59!";
      error = 1;
    }
    if(bd < 0 || bd > 65536) {
      data.message = "Fehler: start delay ! 0-65536!";
      error = 1;
    }
    if(ed < 0 || ed > 65536) {
      data.message = "Fehler: start delay ! 0-65536!";
      error = 1;
    }

    if(error) {
      tpl_editprogram(server, data);
      return;
    }
    else {
      Program p = db.programs[program];
      p.start_hour = bh;
      p.start_min  = bm;
      p.stop_hour  = eh;
      p.stop_min   = em;
      p.start_delay= bd;
      p.stop_delay = ed;
      p.type       = typ;
      ee_wr_program(p);
      data.message = "Programm wurde gespeichert";
      db = ee_getdb();
    }
  }

  data.starthour  = bh;
  data.startmin   = bm;
  data.stophour   = eh;
  data.stopmin    = em;
  data.startdelay = bd;
  data.stopdelay  = ed;
  data.Types[0].value = 0;
  data.Types[0].description = "Astronomisch (Start/Stop Verz&ouml;gerung Einstellen";
  if (typ == 0) {
    data.Types[0].current = "selected='selected'";
  }
  data.Types[1].value = 1;
  data.Types[1].description = "Statisch (Start/Stop Stunde/Minute Einstellen)";
  if (typ == 1) {
    data.Types[1].current = "selected='selected'";
  }

  tpl_editprogram(server, data);
}



void www_air(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  DATA_air data;

  int isactive;
  int tmin;
  int tmax;

  server.httpSuccess();

  if (type == WebServer::POST)  {
    char name[32];
    char value[32];
    bool parsing = true;

    while (parsing) {
      parsing = server.readPOSTparam(name, 32, value, 32);
      if (strcmp(name, "active")  == 0){isactive = atoi(value);}
      if (strcmp(name, "tmin")    == 0){tmin     = atoi(value);}
      if (strcmp(name, "tmax")    == 0){tmax     = atoi(value);}
    }

    if(tmin < 0 || tmin > 65 || tmax < 0 || tmax > 65) {
      data.message = "Fehler: Temperatur 0-65!";
      tpl_air(server, data);
      return;
    }
    else {
      Settings S = db.settings;
      if(isactive) {
	S.aircondition = true;
      }
      else {
	S.aircondition = false;
      }
      S.air_tmin = tmin;
      S.air_tmax = tmax;
      ee_wr_settings(S);
      data.message = "Klimaeinstellungen gespeichert";
      db = ee_getdb();
    }
  }

  data.tmin = db.settings.air_tmin;
  data.tmax = db.settings.air_tmax;

  data.Air[0].id = 0;
  data.Air[0].description = "Aktiv";
  if(db.settings.aircondition == false) {
    data.Air[0].current = "selected='selected'";
  }

  data.Air[1].id = 1;
  data.Air[1].description = "Inaktiv";
  if(db.settings.aircondition == true) {
    data.Air[1].current = "selected='selected'";
  }

  tpl_air(server, data);
}




/*
 * Helper functions
 */

struct DATA_ProgramList prog2web(int p) {
  /*
   * return a human readable version of the
   * given program as a struct, used by webserver
   * templates.
   */
  DATA_ProgramList P;
  P.id = p;
  P.description = "";

  /* FIXME: String concatenation may work incorrectly or crash. Maybe use sprintf() instead */
  if(db.programs[p].type == STATIC) {
    P.description += p + ": Statisch, Start: " + db.programs[p].start_hour + ':' + db.programs[p].start_min;
    P.description += ", Stop: " + db.programs[p].stop_hour + ':' + db.programs[p].stop_min;
    P.modus = "Statisch";
  }
  else {
    P.description = p + ": Astronomisch, Startverschiebung: ";
    P.description += db.programs[p].start_delay + " Minuten, Stopverschiebung: ";
    P.description += db.programs[p].stop_delay + " Minuten";
    P.modus = "Astronomisch";
  }
  return P;
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
    Serial.println("Failed to read from DHT");
  }
  else {
    return t;
  }
}

float humidity() {
  /* read sensor H */
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT");
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
    delay(25);
    digitalWrite(statusled, LOW);
    delay(100);
  }
}

void beep() {
  /* play a C-Dur 4 for 4 milliseconds - a beep */
  tone(speaker, NOTE_C4, 4);
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
      manual = false;
      digitalWrite(mainled, LOW);
    }
    mainstatus = pressed;
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
  for(channel=0; channel<6; channel++) {
    pressed = digitalRead(switches[channel]);
    if(pressed != state[channel] || init) {
      if(manual) {
	if(pressed) {
	  digitalWrite(leds[channel],   HIGH);
	  digitalWrite(relays[channel], LOW);
	}
	else {
	  digitalWrite(leds[channel],   LOW);
	  digitalWrite(relays[channel], HIGH);
	}
      }
      else {
	digitalWrite(leds[channel],   LOW);
      }
      state[channel] = pressed;
    }
  }
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
	if(runtime != state[channel] || init) {
	  if(runtime) {
	    /* within operation time, turn the channel on */
	    digitalWrite(relays[channel], LOW);
	  }
	  else {
	    digitalWrite(relays[channel], HIGH);
	  }
	  state[channel] = runtime;
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

  Serial << "Temperature: " << T << " *C" << endl;
  Serial << "   Humidity: " << h << " %" << endl;
  Serial << hour(t) << ':' << minute(t) << ':' << second(t) << ' ' << day(t) << '.' << month(t) << '.' << year(t) << endl;

  Serial << "   Air Cond: ";
  if(db.settings.aircondition) {
    if(airon) {
      Serial << "running" << endl;
    }
    else {
      Serial << "running" << endl;
    }
    Serial << "       Tmin: " << db.settings.air_tmin << " *C" << endl;
    Serial << "       Tmax: " << db.settings.air_tmax << " *C" << endl;
    Serial << "" << endl;
  }
  else {
    Serial << "turned off" << endl;
  }

  if(dst(t)) {
    Serial << "        DST: Sommerzeit" << endl;
  }
  else {
    Serial << "        DST: Winterzeit" << endl;
  }

  Serial << "    Sunrise: " << sunrisehour << ':' << sunriseminute << endl;
  Serial << "     Sunset: " << sunsethour << ':' << sunsetminute << endl;
}

void sh_channels() {
  /*
   * shows all channel configs with program assignments
   */
  for(int i=0; i<6; i++) {
    Serial << "Channel #" << i << ": " << names[i] << " config: " << endl;
    Serial << "        Program: " << db.channels[i].program << endl;
    Serial << "            Typ: ";
    if(db.programs[db.channels[i].program].type == STATIC) {
      Serial << "Statisch" << endl;
      Serial << "          Start: " << db.programs[db.channels[i].program].start_hour << ':' << db.programs[db.channels[i].program].start_min << endl;
      Serial << "           Stop: " << db.programs[db.channels[i].program].stop_hour << ':' << db.programs[db.channels[i].program].stop_min << endl;
    }
    else {
      Serial << "Astronomisch" << endl;
      Serial << "    Start Delay: " << db.programs[db.channels[i].program].start_delay << " Minuten" << endl;
      Serial << "     Stop Delay: " << db.programs[db.channels[i].program].stop_delay << " Minuten" << endl;
    }
  }
}

void sh_programs() {
  /*
   * show all saved programs in EEPROM
   */
  for(int p=0; p<maxprograms; p++) {
    Serial << "Program #" << p << endl;
    if(db.programs[p].type == STATIC) {
       Serial << "        Typ: Statisch" << endl;
       Serial << "          Start: " << db.programs[p].start_hour << ':' << db.programs[p].start_min << endl;
       Serial << "           Stop: " << db.programs[p].stop_hour << ':' << db.programs[p].stop_min << endl;
    }
    else {
      Serial << "        Typ: Astronomisch" << endl;
      Serial << "    Start Delay: " << db.programs[p].start_delay << " Minuten" << endl;
      Serial << "     Stop Delay: " << db.programs[p].stop_delay << " Minuten" << endl;
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
	Serial << "Error: invalid temperature entered, unallowed char: " << parameter[i] << '!' << endl;
	return;
      }
    }
    if(nvalue != 1) {
      beep();
      Serial << "Error: invalid temperatures entered! Enter 2 digits separated by space (0-65 *C)" << endl;
      return;
    }
    else {
      if(value[0] < 0 || value[0] > 65) {
	beep();
	Serial << "Error: invalid Tmin entered, Tmin " << value[0] << " not within 0-65!" << endl;
	return;
      }
      if(value[1] < 0 || value[1] > 65) {
	beep();
	Serial << "Error: invalid Tmax entered, Tmax " << value[1] << " not within 0-65!" << endl;
	return;
      }

      // if we have done it until here, change the date
      Settings S     = db.settings;
      S.aircondition = true;
      S.air_tmin     = value[0];
      S.air_tmax     = value[1];
      ee_wr_settings(S);
      db = ee_getdb();

      Serial << "Air condition successfully set to Tmin:" << value[0] << ", Tmax: " << value[1] << endl;
      return;
    }
  }
  else {
    // no params given, turn it off
    Settings S     = db.settings;
    S.aircondition = false;
    ee_wr_settings(S);
    db = ee_getdb();

    Serial << "Air condition successfully turned OFF" << endl;
    return;
  }
}


void sh_ip(char parameter[MAXBYTES]) {
  /*
   * sets the ip address based on user serial input
   */
  Serial << "Current IP Address: ";
  Serial << db.settings.octet1 << '.' << db.settings.octet2 << '.' << db.settings.octet3 << '.' << db.settings.octet4 << endl;
  if(parameter[0] != '\0') {
    // ip given, parse it
    int octet[4];
    int noctet;
    char buffer[4];
    byte idx = 0;
    for(int i=0; parameter[i]; i++) {
      if(parameter[i] > '0' || parameter[i] < '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == '.') {
	// one octet done
	octet[noctet] = atoi(buffer);
	noctet++;
	idx = 0;
	buffer[0] = '\0';
      }
      else {
	// no digit, no dot = fail!
	beep();
	Serial << "Error: invalid ip address entered, unallowed char: " << parameter[i] << '!' << endl;
	return;
      }
    }
    if(noctet != 3) {
      beep();
      Serial << "Error: invalid ip address entered! ip must consist of 4 octets separated by . !" << endl;
      return;
    }
    else {
      for(int i=0; i<4; i++) {
	if(octet[i] < 0 || octet[i] > 255) {
	  beep();
	  Serial << "Error: invalid ip address entered, octet " << octet[i] << " not within 0-255!" << endl;
	  return;
	}
      }
      // if we have done it until here, put the octets into eeprom
      Settings S = db.settings;
      S.octet1 = octet[0];
      S.octet2 = octet[1];
      S.octet3 = octet[2];
      S.octet4 = octet[3];
      ee_wr_settings(S);
      db = ee_getdb();
      // FIXME: maybe doesn't work this way, eventually reboot instead, check this!
      uint8_t ip[] = { db.settings.octet1, db.settings.octet2, db.settings.octet3, db.settings.octet4 };
      Ethernet.begin(mac, ip);
      Serial << "IP address successfully changed to " << octet[0] << '.' << octet[1] << '.' << octet[2] << '.' << octet[3] << endl;
      
      return;
    }
  }
}

void sh_setdate(char parameter[MAXBYTES]) {
  /*
   * sets the date based on user serial input
   */
  if(parameter[0] != '\0') {
    // date given, parse it
    int value[3];
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
	Serial << "Error: invalid date entered, unallowed char: " << parameter[i] << '!' << endl;
	return;
      }
    }
    if(nvalue != 2) {
      beep();
      Serial << "Error: invalid date entered! date must consist of 3 digits separated by . !" << endl;
      return;
    }
    else {
      if(value[0] < 1 || value[0] > 31) {
	beep();
	Serial << "Error: invalid day entered, day " << value[0] << " not within 1-31!" << endl;
	return;
      }
      if(value[1] < 1 || value[1] > 12) {
	beep();
	Serial << "Error: invalid month entered, month " << value[1] << " not within 1-12!" << endl;
	return;
      }
      if(value[2] < 2010 || value[2] > 3600) {
	beep();
	Serial << "Error: invalid year entered, year " << value[2] << " not within 2010-3600!" << endl;
	return;
      }

      // if we have done it until here, change the date
      t  = gettimeofday();
      int stunde = hour(t);
      int min    = minute(t);
      int sekunde= second(t);
      setTime(stunde, min, sekunde, value[0], value[1], value[2]);

      Serial << "Date successfully changed to " << value[0] << '.' << value[1] << '.' << value[2] << endl;
      return;
    }
  }
  else {
    beep();
    Serial << "Error: Parameter in the form DD.MM.YYYY missing!" << endl;
  }
}

void sh_settime(char parameter[MAXBYTES]) {
  /*
   * sets the time based on user serial input
   */
  if(parameter[0] != '\0') {
    // time given, parse it
    int value[3];
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
	Serial << "Error: invalid time entered, unallowed char: " << parameter[i] << '!' << endl;
	return;
      }
    }
    if(nvalue != 2) {
      beep();
      Serial << "Error: invalid time entered! time must consist of 3 digits separated by : !" << endl;
      return;
    }
    else {
      if(value[0] < 0 || value[0] > 23) {
	beep();
	Serial << "Error: invalid hour entered, hour " << value[0] << " not within 0-23!" << endl;
	return;
      }
      if(value[1] < 0 || value[1] > 59) {
	beep();
	Serial << "Error: invalid minute entered, minute " << value[1] << " not within 0-59!" << endl;
	return;
      }
      if(value[2] < 0 || value[2] > 59) {
	beep();
	Serial << "Error: invalid second entered, second " << value[2] << " not within 0-59!" << endl;
	return;
      }

      // if we have done it until here, change the date
      t  = gettimeofday();
      int tag    = day(t);
      int monat  = month(t);
      int jahr   = year(t);
      setTime(value[0], value[1], value[2], tag, monat, jahr);

      Serial << "Time successfully changed to " << value[0] << '.' << value[1] << '.' << value[2] << endl;
      return;
    }
  }
  else {
    beep();
    Serial << "Error: Parameter in the form HH:MM:SS missing!" << endl;
  }
}

void sh_pins() {
  /*
   * Display a table of our PINs with their current state
   */
  t = gettimeofday();
  Serial << "Current PIN states:" << endl << endl;
  Serial << "Switch | MAIN | Ch1 | Ch2 | Ch3 | Ch4 | Ch5 | Ch6" << endl;
  Serial << "-------+------+-----+-----+-----+-----+-----+-----" << endl;
  Serial << "  PIN  | " << mainswitch << "  ";
  for (int i=0; i<6; i++) {
    Serial << "|  " << switches[i] << " ";
  }
  Serial << endl;
  Serial << "-------+------+-----+-----+-----+-----+-----+-----" << endl;
  Serial << " Mode  |   " << manual << "  ";
  for (int i=0; i<6; i++) {
    Serial << "|   " << state[i] << " ";
  }
  Serial << endl;
  Serial << "-------+------+-----+-----+-----+-----+-----+-----" << endl;
  Serial << " Relay |      ";
  for (int i=0; i<6; i++) {
    int ops = operate(i, t);
    Serial << "|   " << ops << " ";
  }
  Serial << endl << endl;
}

void check_command(int command, char parameter[MAXBYTES]) {
  /*
   * Execute command taken from serial port
   */
  if (command == 'r') {
    Serial << "Resetting Config" << endl;
    ee_init(INIT);
    Serial << "Initiating a soft reset" << endl;
    software_Reset();
  }
  else if(command == 'b') {
    Serial << "Initiating a soft reset" << endl;
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
  else if(command == 'h' || command == '?') {
    Serial << "Available commands: " << endl;
    Serial << "  r            - reset EEPROM configuration to defaults and do a soft reset" << endl;
    Serial << "  b            - do a soft reset" << endl;
    Serial << "  s            - display T and H sensor data and date+time" << endl;
    Serial << "  i            - show ip address" << endl;
    Serial << "  i x.x.x.x    - set ip address" << endl;
    Serial << "  d DD.MM.YYYY - set date" << endl;
    Serial << "  t HH:MM:SS   - set time (enter wintertime!)" << endl;
    Serial << "  a Tmin Tmax  - set aircondition, leave params to turn it off" << endl;
    Serial << "  c            - dump channel config" << endl;
    Serial << "  p            - dump program config" << endl;
    Serial << "  P            - dump PIN states" << endl;
    Serial << "  h            - show help" << endl;
  }
  else {
    beep();
    Serial.println("Invalid Command (try 'h' to get help)!");
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
      command      = 0;
    }
    else if(onebyte == ' ') {
      parametermode = true;
    }
    else {
      if(parametermode) {
	if(index == MAXBYTES) {
	  beep();
	  Serial << "Error: too many chars entered. Try Again!" << endl;
	  index        = 0;
	  parameter[0] = '\0';
	  command      = 0;
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
  pinMode(statusled,  OUTPUT);

  blink();
  Serial.begin(9600);

  blink();
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");

  t = now();

  blink();
  dht.begin();

  blink();
  pinMode(speaker,    OUTPUT); 
  pinMode(air,        OUTPUT);
  check_air(INIT);

  blink();
  pinMode(mainswitch, INPUT);
  pinMode(mainled,    OUTPUT);
  check_main(INIT);

  for(int i=0; i<6; i++) {
    blink();
    pinMode(switches[i], INPUT);
    pinMode(leds[i],     OUTPUT);
    pinMode(relays[i],   OUTPUT);
  }
  check_switches(INIT);

  blink();
  check_timers(INIT);

  blink();
  Wire.begin();

  blink();
  ee_init(RUN);
  db = ee_getdb();

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
  webserver.begin();

  /* booting done, keep status on */
  digitalWrite(statusled, HIGH);
  beep();
}


void loop() {
  check_switches(RUN);
  check_timers(RUN);
  check_air(RUN);
  check_shell();
  webserver.processConnection();
}
