#include <stdlib.h>
#include <Arduino.h>




#define AUTO   1    /* use astronomical start/stop time */
#define MANUAL 2    /* user specified start/stop times  */

const int START_CHANNELS = 0;
const int START_PROGRAMS = 256;
const int SIZE_CHANNELS  = 8;
const int SIZE_PROGRAMS  = 32;

/*
 * record for time programs
 */
struct Program {
  int type;            // auto or manual
  int start_delay;     // minutes to add/substract from astro start
  int stop_delay;      // minutes to add/substract from astro stop
  int start_hour;      // manual start, HH
  int start_min;       // manual start, MM
  int stop_hour;       // manual stop, HH
  int stop_min;        // manual stop, MM
  int res1;
  int res2;
  int res3;
  int res4;
};

struct Channel {
  int id;
  int program;
  int res1;
  int res2;
  int res3;
  int res4;
};



class Data {
 public:
  Data();
  struct Channel getchannel(int address);
  struct Program getprogram(int address);
  bool isempty();
  void init();


 private:
  Channel channel[SIZE_CHANNELS];
  Program program[SIZE_PROGRAMS];

  EDB channeldb();
  EDB programdb();

};


int test();
