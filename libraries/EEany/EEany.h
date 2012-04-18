#ifndef EEANY
#define EEANY

#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

struct Header {
  int version;
};

struct Settings {
  bool dst; // daylight savings time
  int  tz;  // timezone
  bool aircondition;
  bool b2;
  bool b3;
  bool b4;
  uint8_t octet1;
  uint8_t octet2;
  uint8_t octet3;
  uint8_t octet4;
  int  air_tmin;
  int  air_tmax;
  int  i3;
  int  i4;
};

struct Channel {
  int id;
  int program;
  int res1;
  int res2;
  int res3;
  int res4;
};

struct Program {
  int id;
  int type;            // auto or manual
  int start_delay;     // minutes to add/substract from astro start
  int stop_delay;      // minutes to add/substract from astro stop
  int start_hour;      // manual start, HH
  int start_min;       // manual start, MM
  int stop_hour;       // manual stop, HH
  int stop_min;        // manual stop, MM
  int cooldown;
  int res2;
  int res3;
  int res4;
};

struct Database {
  Header   header;
  Settings settings;
  Channel  channels[8];
  Program  programs[32];
};

extern int dbversion;
extern int maxchannels;
extern int maxprograms;

bool         ee_exists();
Database     ee_getdb();
unsigned int ee_init(bool force);
unsigned int ee_wr_settings(Settings s);
unsigned int ee_wr_channel(Channel c);
unsigned int ee_wr_program(Program p);


#endif
