#include <EEPROM.h>
#include "EEany.h"
#include <Wire.h>

int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1000;

void pr(char type, int num, int written) {
  Serial.print("Wrote ");
  
  switch(type) {
    case 'p': Serial.print(" program: "); break;
    case 'c': Serial.print(" channel: "); break;
    case 's': Serial.print("settings: "); break;
  }
  
  Serial.print(num);
  Serial.print(" with ");
  Serial.print(written);
  Serial.print(" bytes");
  Serial.println();
}


void setup() {
 Serial.begin(9600);

 Wire.begin();
 unsigned int written = ee_init(false);
 
 delay(2000);
 Serial.println("Writing Database to EEprom");
 
 Database db = ee_getdb();
 
 int startd = 10;
 int stopd  = 10;
 
 for (int i=0; i<8; i++) {
   Program p;
   p.id   = i;
   p.type = 1;
   p.start_delay = startd;
   p.stop_delay  = stopd;
   written = ee_wr_program(p);
   pr('p', i, written);
   
   Channel c;
   c.id      = i;
   c.program = i;
   written = ee_wr_channel(c);
   pr('c', i, written);
   
   startd += 12;
   stopd  += 14;
 }
 
 Settings s;
 s.dst = false;
 s.tz  = 2;
 written = ee_wr_settings(s);
 pr('s', 0, written);
}

void loop () {}

