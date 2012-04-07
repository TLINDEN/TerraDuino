#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

void setup()  {
  Serial.begin(9600);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");      
}

void loop()
{
   digitalClockDisplay();  
   delay(1000);
}

void digitalClockDisplay(){
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

void printDigits(int digits){
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

