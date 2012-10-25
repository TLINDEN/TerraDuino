/* -*-c-*-
 * Terraduino2 Terrarium Controller with Arduino.
 *
 * Copyright (c) 2012 Thomas Linden
 *
 */

//#include <MemoryFree.h>

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
#include "EthernetClient.h"

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

// if running via macbook or via router, uncomment to run via router
//#define LOCAL

/* global vars */
time_t t = 0;
time_t booted = 0;
long uptime = 0;
int startdelay = 0;
int stopdelay = 0;
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t mask[] = { 255, 255, 255, 0 };
uint8_t ip[4];
uint8_t gw[4];
const char endl[3] = "\r\n";
bool manual = false;
uint8_t mainstatus = 0;
uint8_t pressed    = 0;
uint8_t swpressed  = 0;
unsigned long timer = 0;
unsigned long moment = 0;
unsigned long sttimer = 0;
unsigned long stmoment = 0;
unsigned long airtimer = 0;
unsigned long airmoment = 0;
bool airon = false;
unsigned long alarmpaused = 0;

unsigned long djtimer = 0;
unsigned long djmoment = 0;

const unsigned long alarmwait        = 120;    // min, 2 hours
const unsigned long timerintervall   = 5000;   // ms,  5 sec;
const unsigned long airtimerinterval = 720000; // ms,  10 min
const unsigned long djtimerintervall = 360000;  // ms,  orig: 360000 - 5 min

uint8_t channel = 0;
uint8_t program = 0;
bool INIT = true;
bool RUN  = false;
uint8_t runtime = 0;
long cooldown[] = {0, 0, 0, 0, 0, 0, 0};
long dbcooldown[] = {0, 0, 0, 0, 0, 0, 0};
int begin = 0;
int end = 0;

// serial parser vars
int     value[8];
uint8_t nvalue = 0;
char    buffer[5];
byte    idx = 0;

    
uint8_t onebyte = 0;
uint8_t command = 0;
char parameter[MAXBYTES];
byte index = 0;
bool parametermode = false;
char digits[3];

/* important for EEany initialization */
int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1001;

/* post parser vars */
char parsename[32];
char parsevalue[32];
bool parsing = true;

/* PINs */
const uint8_t switches[] = { 35, 49, 47, 45, 43, 41 }; // 1st 15
const uint8_t leds[]     = { 33, 48, 46, 44, 42, 40 }; // 1st 16
const uint8_t relays[]   = { 36, 34, 32, 30, 28, 26, 22 };
uint8_t state[]          = { 0, 0, 0, 0, 0, 0 };
uint8_t runstate[]       = { 0, 0, 0, 0, 0, 0 };

/* names for the channels */
const char* names[] = { "70 W Links", "70 W Rechts", "40 W Vorn", "40 W Hinten", "15 W Links", "15 W Rechts", "Reserve"};

const uint8_t air        = 24;
const uint8_t statusled  = 27;
const uint8_t speaker    = 37; //17;
const uint8_t mainswitch = 29; // 19
const uint8_t mainled    = 31; // 18

const uint8_t numchannels = 7;
const uint8_t numswitches = 6;
uint8_t i = 0;

const uint8_t dayspermonth[] = {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* global objects */
DHT dht(DHTPIN, DHTTYPE);
Database db;

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

IPAddress djserver(212,227,251,58);
//IPAddress djserver(192,168,128,1);
const char djhostname[] = "www.daemon.de";
EthernetClient djclient;
char djuri[80];
byte named[] = { 141,1,1,1 };

// helpers for djput+get
char data[512];
int datacount = 0;
char line[80];
bool changed = false;
Program plist[8];
Channel clist[8];
int pcount = 0;
int ccount = 0;
char c = 0;
char supti[32];
char shumi[16];
char stemp[16];
char stime[32];

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

void getcooldown() {
  /*
   * store configured cooldown time in dbcooldown array
   * pre-calculated in milliseconds, so we don't have
   * to do conversion calculations on each loop
   */
  for(channel=0; channel<maxchannels; channel++) {
    dbcooldown[channel] = db.programs[db.channels[channel].program].cooldown * 60000;
  }
}

void reset_eth() {
  delay(5);
  Ethernet.begin(mac, ip, named, gw, mask);
}

void djput (char djuri[80]) {
  Serial << "djput: " << djuri << endl;
  if (djclient.connect(djserver, 80)) {
    djclient.print("GET ");
    djclient.print(djuri);
    djclient.println(" HTTP/1.0");
    djclient.print("Host: ");
    djclient.println(djhostname);
    djclient.println("Connection: close");
    djclient.println();
    //Serial << "headers sent" << endl;
    //while(djclient.connected()) {
      //Serial << "Still connected" << endl;
      //while(djclient.available()) {
        // we've come so far, abort, we're not interested in any output for now
        //djclient.stop();
	//break;
      //}
      //Serial << endl << "Done reading for now..." << endl;
    //}
    //Serial << "no more connected" << endl;
    djclient.flush();
    djclient.stop();
    //Serial << "stopped" << endl;
 } 
 else {
    // if you couldn't make a connection:
    //Serial.println("connection failed");
   djclient.flush();
   djclient.stop();
  }
}



void djcheckmodified (char djuri[80]) {
 /*
  * Connect to django and fetch modified channel and program configs, if any.
  * Output will be in csv format, parse it and store changed channels and programs
  * in temporary lists during parsing, which leads to faster disconnect from http.
  * after the disconnect all changed channels and programs will be saved to eeprom
  * individually, the db fetched back to memory and cooldowns updated.
  */
  Serial << "djcheckmodified: " << djuri << endl;
  if (djclient.connect(djserver, 80)) {
    //Serial << "Connected" << endl;
    djclient.print("GET ");
    djclient.print(djuri);
    djclient.println(" HTTP/1.0");
    djclient.print("Host: ");
    djclient.println(djhostname);
    djclient.println("Connection: close");
    djclient.println();
    
    data[0] = '\0';
    datacount = 0;
    line[0] = '\0';
    index = 0;
    
    changed = false;
    
    plist[0].id = 0;
    clist[0].id = 0;
    
    pcount = 0;
    ccount = 0;
    c = 0;
    
    while(djclient.connected()) {
      //Serial << "Still connected" << endl;
      while(djclient.available()) {
        if(datacount == 511) {
          break;
        }
        else {
          data[datacount] = djclient.read();
          //Serial.write(data[datacount]);
          datacount++;
        }
      }
      //Serial << "Done reading for now..." << endl;
    }
    djclient.flush();
    djclient.stop();
    data[datacount] = '\0';
    //Serial << "Got: " << datacount << " chars of: "  << endl << data << endl;
    
    for(int pos=0; pos < datacount; pos++) {
        c = data[pos];
        if(c == '\n') {
          // line complete
          if(line[0] == 'c') {
           // channel 
           //Serial << "Channel: " << line << endl;
           sscanf(line, "c,%d,%d", &post.i1, &post.i2);
           Channel ch = db.channels[post.i1];
           ch.program = post.i2;
           //Serial << "Parsed Channel " << post.i1 << " using program " << post.i2 << endl;
           changed = true;
           clist[ccount] = ch;
           ccount++;
           resetpost();
          }
          else if(line[0] == 'p') {
           // program 
           //Serial << "Program: " << line << endl;
           // p,6,0,0,0,0,0,0,0,0,0,0,1
           sscanf(line, "p,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &post.i1, &post.i2, &post.i3, &post.i4, &post.i5,
                                          &post.i6, &post.i7, &post.i8, &post.i9, &post.i10, &post.i11, &post.i12);
           Program p = db.programs[post.i1];
           p.type       = post.i12;
           p.start_hour = post.i4;
           p.start_min  = post.i5;
           p.stop_hour  = post.i6;
           p.stop_min   = post.i7;
           p.start_delay= post.i2;
           p.stop_delay = post.i3;
           p.cooldown   = post.i8;
           p.sleep_day  = post.i9;
           p.sleep_mon  = post.i10;
           p.sleep_increment  = post.i11;
           //Serial << "Parsed Program " << post.i1 << ": type " << p.type << ", start delay " << p.start_delay << ", stop delay: " << p.stop_delay << endl;
           changed = true;
           plist[pcount] = p;
           pcount++;
           resetpost();
          }
          index = 0;
          line[index] = '\0';
        }
        else if (c != '\r') {
          // regular char no beginning of newline
          line[index] = c;
          index++;
          line[index] = '\0';
        }
      }

    if(changed) {
      for (index=0; index < ccount; index++) {
        ee_wr_channel(clist[index]);
        delay(10);
      }
      for (index=0; index < pcount; index++) {
        ee_wr_program(plist[index]);
        delay(10);
      }
      db = ee_getdb();
      getcooldown();
    }

    resetpost();
    //Serial << "done with djget()" << endl;
 } 
 else {
   //Serial.println("connection failed");
    djclient.flush();
    djclient.stop();
  }
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

int coperate(int channel, time_t t) {
 if(db.programs[db.channels[channel].program].type == MANUAL || manual) {
   return state[channel];
 }
 
 if(operate(channel, t)) {
  if( db.programs[db.channels[channel].program].cooldown > 0) {
    if(cooldown[channel] < dbcooldown[channel]) {
      return LOW;
    }
    else {
      return HIGH;
    }
  }
  else {
   return HIGH; 
  }
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


int freeRam () {
  /*
   * source: http://jeelabs.org/2011/05/22/atmega-memory-use/
   */
  extern int __heap_start, *__brkval; 
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


char slash[2] = "/";
char dj[][14] = { "0", "0", "0", "0", "0", "0", "0", "0", "00.00", "00.00", "0000000000", "0000000000", "0000", "0000" };

void check_report(bool init) {
  /*
   * Posts report and status data every 5 minutes to django & look if some config has been changed
   *
   * URL zum posten der report und status daten:
   * old: http://www.daemon.de/td/0/0/0/0/0/0/0/0/23/44/1349439160/4500/123122/
   * new: http://www.daemon.de/td/0/0/0/0/0/0/0/0/23/44/1349439160/123122/530/1920/
   * Felder: c1, c2, c3, c4, c5, c6, c7, c8, t, h, ts, uptime, sunrise, sunset
   *
   */
   djmoment = millis();
   if(init || djmoment - djtimer > djtimerintervall) {
     t = gettimeofday();

     begin = getsunrise(t); // int minutes
     delay(1);
     end   = getsunset(t);

     for (channel=0; channel<numchannels; channel++) {
       itoa(coperate(channel, t), dj[channel], 10);
     }

     dtostrf(temperature(), 4, 2, dj[8]);
     dtostrf(humidity(),    4, 2, dj[9]);
     dtostrf(t,            10, 0, dj[10]);
     dtostrf(uptime,        1, 0, dj[11]);
     dtostrf(begin,         3, 0, dj[12]);
     dtostrf(end,           3, 0, dj[13]);
     djuri[0] = '\0';
     strncat(djuri, "/td/", 4);
     for (channel=0; channel<14; channel++) {
       strncat(djuri, dj[channel], strlen(dj[channel]));
       strncat(djuri, slash, 1);
     }
     djput(djuri);

     delay(10);
     djuri[0] = '\0';
     strncat(djuri, "/td/modified/", 13);
     djcheckmodified(djuri);
     djuri[0] = '\0';
     
     djtimer = djmoment;
     
     // we reset the eth after every loop to clean up internal buffers
     // to avoid hanging after a while
     //reset_eth();
   }
}


void check_reboot() {
  if ( uptime > 10000 && hour(t) == 0 ) {
    // midnight. reboot.
    software_Reset();
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

  for(channel=0; channel<numswitches; channel++) {
    swpressed = digitalRead(switches[channel]);

    if(dbcooldown[channel] > 0) {           
      // program has cooldown time configured
      if(swpressed && (manual || db.programs[db.channels[channel].program].type == MANUAL)) {       
	// proposed runtime                          
	if(cooldown[channel] > dbcooldown[channel]) {
	  // was long enough off, don't wait anymore, turn it on
	  state[channel] = 0;
	}    
	else {                             
	  // wait more, time not elapsed yet   
	  cooldown[channel] += (stmoment - sttimer);
	}
      }    
      else {          
	// shall be off                           
	if(cooldown[channel] < dbcooldown[channel]) {
	  // still time left, add to cooldown more
	  cooldown[channel] += (stmoment - sttimer); 
	}
      } 
    }

    if(swpressed != state[channel] || init) {
      if(manual || db.programs[db.channels[channel].program].type == MANUAL) {
	if(swpressed) {
	  digitalWrite(leds[channel],   HIGH);
	  if(dbcooldown[channel]) {
	    if(cooldown[channel] > dbcooldown[channel]) {
	      // ok, it's cooled down enough, switch it
	      digitalWrite(relays[channel], LOW);
	    }
            else {
              // nope, keep it off
              digitalWrite(relays[channel], HIGH);
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

    if(init || moment - timer > timerintervall) {
      t = gettimeofday();
      for(channel=0; channel<numchannels; channel++) {
        if(db.programs[db.channels[channel].program].type == MANUAL) {
          continue; 
        }
        
	runtime = operate(channel, t);
        
        if(dbcooldown[channel] > 0) {
          // program has cooldown time configured
          if(runtime) {
            // proposed runtime
            if(cooldown[channel] > dbcooldown[channel]) {
              // was long enough off, don't wait anymore, turn it on
              runstate[channel] = 0;
            }
            else {
              // wait more, time not elapsed yet
              cooldown[channel] += (moment - timer);
            }
          }
          else {
            // shall be off
            if(cooldown[channel] < dbcooldown[channel]) {
              // still time left, add to cooldown more
              cooldown[channel] += (moment - timer);
            }
          }
        }
	if(runtime != runstate[channel] || init) {
	  if(runtime) {
	    /* within operation time, turn the channel on */
	    if(dbcooldown[channel] > 0) {
	      if(cooldown[channel]  > dbcooldown[channel]) {
		// ok, it's cooled down enough, switch it
		digitalWrite(relays[channel], LOW);
	      }
              else {
                // nope, keep it off
                digitalWrite(relays[channel], HIGH);
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
    nvalue = 0;
    idx = 0;
    buffer[0] = '\0';
    for(i=0; parameter[i]; i++) {
      if(parameter[i] > '0' || parameter[i] < '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == ' ') {
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
    buffer[0] = '\0';
    for(i=0; parameter[i]; i++) {
      if(parameter[i] >= '0' && parameter[i] <= '9') {
	// a digit
	buffer[idx] = parameter[i];
	idx++;
	buffer[idx] = '\0';
      }
      else if(parameter[i] == '.' || parameter[i] == ' ') {
	// one octet done or ip done, begin of gw
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
    
    if(nvalue != 8) {
      beep();
      Serial << f_sherr_form << f_sherr_exip << endl;
      return;
    }
    else {
      for(i=0; i<8; i++) {
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
      S.gw1 = value[3];
      S.gw2 = value[4];
      S.gw3 = value[5];
      S.gw4 = value[6];
      ee_wr_settings(S);
      db = ee_getdb();
      ip[0] = db.settings.octet1;
      ip[1] = db.settings.octet2;
      ip[2] = db.settings.octet3;
      ip[3] = db.settings.octet4;  
      gw[0] = db.settings.gw1;
      gw[1] = db.settings.gw2;
      gw[2] = db.settings.gw3;
      gw[3] = db.settings.gw4;
      reset_eth();
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
    buffer[0] = '\0';
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
    nvalue = 0;
    idx = 0;
    buffer[0] = '\0';
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
    Serial << f_mem <<  freeRam() << endl;
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
        
        if(operate(0, t) || operate(1, t)) {
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
     Serial << f_init << f_rtc_ok << endl;

  t = now();
  
  blink();
  Serial << f_init << f_init_dht << endl;
  dht.begin();

  blink();
  Serial << f_init << f_init_wire << endl;
  Wire.begin();

  blink();
  Serial << f_init << f_init_eep << endl;
  ee_init(RUN);
  
  blink();
  Serial << f_init_dbread << endl;
  db = ee_getdb();
  getcooldown();

  if(dbversion > db.header.version) {
    alarm();
    Serial << f_init_dberror << endl;
    ee_init(INIT);
    db = ee_getdb();    
  }
  else {
    Serial << f_init_dbok << db.header.version << endl;
  }
  
  blink();
  Serial << f_init << f_init_speak << endl;
  pinMode(speaker,    OUTPUT); 
  
  blink();
  Serial << f_init << f_init_air << endl;
  pinMode(air,        OUTPUT);
  digitalWrite(air, HIGH);
  delay(5);
  check_air(INIT);
  
  blink();
  Serial << f_init << f_init_sw << "M ";
  pinMode(mainswitch, INPUT);
  pinMode(mainled,    OUTPUT);

  check_main(INIT);
  
  for(channel=0; channel<numswitches; channel++) {
    blink();
    Serial << channel << ' ';
    pinMode(switches[channel], INPUT);
    pinMode(leds[channel],     OUTPUT);
  }
  Serial << endl;
 
  blink();
  Serial << f_init << f_init_relay; 
  for(channel=0; channel<numchannels; channel++) {
    pinMode(relays[channel],   OUTPUT);
    Serial << channel << ' ';
  }
  Serial << f_init_ok << endl;

  blink();
  Serial << f_init << f_init_timers << endl;
  check_timers(INIT);

  blink();
  Serial << f_init << f_init_eth << db.settings.octet1 << '.' << db.settings.octet2 << '.' << db.settings.octet3 << '.' << db.settings.octet4 << endl;
#ifndef LOCAL
  ip[0] = db.settings.octet1;
  ip[1] = db.settings.octet2;
  ip[2] = db.settings.octet3;
  ip[3] = db.settings.octet4;
  gw[0] = db.settings.gw1;
  gw[1] = db.settings.gw2;
  gw[2] = db.settings.gw3;
  gw[3] = db.settings.gw4;
#else
  ip[0] = 10;
  ip[1] = 1;
  ip[2] = 1;
  ip[3] = 2;
  gw[0] = 10;
  gw[1] = 1;
  gw[2] = 1;
  gw[3] = 1;
#endif
  reset_eth();

  
  /* finally enable watchdog and set PAT timeout */
  blink();
  Serial << f_init << f_init_wdt << WDTO_4S << 's' << endl;
  wdt_enable(WDTO_4S);

  /* booting done, keep status on */
  digitalWrite(statusled, HIGH);
  beep();
  Serial << f_mem <<  freeRam() << endl << f_init_done << endl << endl;
  Serial << f_prompt;
  
  booted = gettimeofday();
}


void loop() {
  check_main(RUN);
  check_switches(RUN);
  check_timers(RUN);
  check_air(RUN);
  check_shell();
  check_report(RUN);
  wdt_reset();
}

