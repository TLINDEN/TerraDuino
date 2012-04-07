

#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h> 

#include "Flash.h"
#include "Sunrise.h"

int relais = 36;

time_t today;
int startdelay;
int stopdelay;

bool dst(time_t t) {
  /*
   * calculate daylight savings time
   *
   * via: http://www.mikrocontroller.net/attachment/8391/TIME.C
   */
  int monat     = month(t);
  int tag       = day(t);
  int wochentag = weekday(t);
  int stunde    = hour(t);
  
  if( monat < 3 || monat > 10 )	{
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
  time_t t = now();
  if(dst(t)) {
    return t + 3600; 
  }
  else {
    return t;
  }
}

void checkschaltzeit(time_t t) {
      int monat = month(t);
      int tag   = day(t);
      
      int h;
      int m;
      int cur;
      
      int begin = sunrise[monat - 1][tag - 1];
      delay(1);
      int end = sunrise[monat + 11][tag - 1];
      
      if(dst(t)) {
        begin += 60;
        end   += 60;
      }
      
      begin += startdelay;
      end   -= stopdelay;
      
      Serial.print(", Sonnenaufgang: ");
      h = (begin - (begin % 60)) / 60;
      m = begin % 60;
      printDigits(h);
      Serial.print(":");
      printDigits(m);
      Serial.print(", Sonnenuntergang: ");
      h = (end - (end % 60)) / 60;
      m = end % 60;
      printDigits(h);
      Serial.print(":");
      printDigits(m);
      
      h = hour(t);
      m = minute(t);
      
      cur = (h *60) + m;
      
      if(cur >= begin && cur <= end) {
        Serial.print("  EIN ");
        digitalWrite(relais, LOW);
      }
      else {
        Serial.print("  AUS  "); 
        digitalWrite(relais, HIGH);
      }
      
      Serial.println();
}

void digitalClockDisplay(time_t t){
  printDigits(hour(t));
  Serial.print(':');
  printDigits(minute(t));
  Serial.print(':');
  printDigits(second(t));
  
  Serial.print(" ");
  printDigits(day(t));
  Serial.print('.');
  printDigits(month(t));
  Serial.print('.');
  Serial.print(year(t));
  /*
  Serial.print("   time_t: ");
  Serial.print(t);
  */
}

void printDigits(int digits){
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void setup() {
  Serial.begin(9600);
    setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");

  today = now();
  
  startdelay = 690;
  stopdelay  = 110;
  
  pinMode(relais, OUTPUT);
}

void loop() {
  today = gettimeofday();
  digitalClockDisplay(today);
  checkschaltzeit(today);
  delay(1000);
}
