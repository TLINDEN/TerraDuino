#include <EEPROM.h>
#include "EEany.h"
#include <Wire.h>

int maxchannels = 8;
int maxprograms = 32;
int dbversion   = 1000;

char endl = '\n';
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }


void setup() {
 Serial.begin(9600);
 Wire.begin();
 unsigned int written = ee_init(false);

 delay(2000);
 Serial << "Reading Database" << endl;
 
 Database db = ee_getdb();
 
 Serial << "    Got DB Version " << db.header.version << endl;

 for(int i=0; i<8; i++) {
   Serial << "Channel " << i << " assigned program: " << db.channels[i].program << endl;
   Serial << "     Startdelay: " << db.programs[db.channels[i].program].start_delay << ", Stopdelay: " << db.programs[db.channels[i].program].stop_delay << endl;
 }

 Serial << endl << "Settings: Timezone: " << db.settings.tz << ", DST: " << db.settings.dst << endl;
}

void loop () {}

