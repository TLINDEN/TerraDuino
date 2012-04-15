#include <EEPROM.h>
#include "EEany.h"
#include <Wire.h>


void setup() {
 Serial.begin(9600);
 Wire.begin();
 
 unsigned int written = ee_init(true);
 
 Serial.print("bytes written: ");
 Serial.print(written);
 Serial.println();
 
 Database db = ee_getdb();

 Serial.print("channel id 4 assigned program: ");
 Serial.print(db.channels[4].program);
 Serial.println();
}

void loop () {}

