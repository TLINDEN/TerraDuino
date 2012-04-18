#include "EEany.h"


bool ee_exists() {
  Header header;
  EEPROM_readAnything(0, header);
  if(header.version == dbversion) {
    return true;
  }
  else {
    return false;
  }
}

Database ee_getdb() {
  Database db;
  EEPROM_readAnything(0, db);
  return db;
}

unsigned int ee_init(bool force) {
  unsigned int written;
  Database db;

  if(ee_exists() && !force ) {
    /* only initialize the db, if empty or forced */
    return 0;
  }

  db.header.version = dbversion;

  db.settings.dst = true;
  db.settings.tz  = 2;
  db.settings.aircondition  = false;
  db.settings.b2  = false;
  db.settings.b3  = false;
  db.settings.b4  = false;
  db.settings.air_tmin  = 0;
  db.settings.air_tmin  = 0;
  db.settings.i3  = 0;
  db.settings.i4  = 0;
  db.settings.octet1 = 10;
  db.settings.octet2 = 0;
  db.settings.octet3 = 0;
  db.settings.octet4 = 1;

  for (int cid=0; cid<maxchannels; cid++) {
    db.channels[cid].id      = cid;
    db.channels[cid].program = cid;
    db.channels[cid].res1    = 0;
    db.channels[cid].res2    = 0;
    db.channels[cid].res3    = 0;
    db.channels[cid].res4    = 0;
  }
  

  for (int pid=0; pid<maxprograms; pid++) {
    db.programs[pid].id = pid;
    db.programs[pid].type = 1;
    db.programs[pid].start_delay = 0;
    db.programs[pid].stop_delay = 0;
    db.programs[pid].start_hour = 0;
    db.programs[pid].start_min = 0;
    db.programs[pid].stop_hour = 0;
    db.programs[pid].stop_min = 0;
    db.programs[pid].cooldown = 0;
    db.programs[pid].res2 = 0;
    db.programs[pid].res3 = 0;
    db.programs[pid].res4 = 0;
  }

  written = EEPROM_writeAnything(0, db);

  return written;
}

unsigned int ee_wr_settings(Settings s) {
  int address = sizeof(Header);
  return EEPROM_writeAnything(address, s);
}

unsigned int ee_wr_channel(Channel c) {
  int address = sizeof(Header) + sizeof(Settings) + (sizeof(Channel) * c.id);
  return EEPROM_writeAnything(address, c);
}

unsigned int ee_wr_program(Program p) {
  int address = sizeof(Header) + sizeof(Settings) + (sizeof(Channel) * maxchannels) + (sizeof(Program) * p.id);
  return EEPROM_writeAnything(address, p);
}
