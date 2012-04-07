
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h> 
#include "Sunrise.h"


/*
 * check if now is between start and stop time
 */
bool runtime(int start, int stop, int current) {
 if(current >= start && current <= stop) {
  return true;
 }
 else {
  return false;
 } 
}

void digitalClockDisplayNow(){
  printDigits(hour());
  Serial.print(':');
  printDigits(minute());
  Serial.print(':');
  printDigits(second());
  
  Serial.print(" ");
  printDigits(day());
  Serial.print('.');
  printDigits(month());
  Serial.print('.');
  Serial.print(year());
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
  Serial.print("   time_t: ");
  Serial.print(t);
  Serial.println(); 
}

void printDigits(int digits){
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

time_t today;
int aufgang;

void setup() {
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");
     
  Serial.print("Datum: ");
  today = now();
  digitalClockDisplay(today);

  Serial.println("init sunrises");
  initsunrises();
  Serial.println("done");
}

void loop() {
  /*
  aufgang = sunrise[month(today)][day(today)][AUFGANG];
  Serial.print("Sonnenaufgang: ");
  Serial.print(aufgang);
  Serial.println();
  */
  Serial.print("        Datum: ");
  today = now();
  digitalClockDisplay(today);
  
  Serial.println();
  Serial.println();
  delay(3000);
}

