
#include "Flash.h"
#include "Sunrise.h"


void prsunrise(int tag, int monat) {
      int h;
      int m;
      int begin = sunrise[monat - 1][tag - 1];
      delay(1);
      int end   = sunrise[monat + 11][tag - 1];
      
      Serial.print(tag);
      Serial.print(".");
      Serial.print(monat);
      Serial.print(". - Sonnenaufgang: ");
      h = (begin - (begin % 60)) / 60;
      m = begin % 60;
      Serial.print(h);
      Serial.print(":");
      Serial.print(m);
      Serial.print(", Sonnenuntergang: ");
      h = (end - (end % 60)) / 60;
      m = end % 60;
      Serial.print(h);
      Serial.print(":");
      Serial.print(m);
      Serial.println();
}

void printDigits(int digits){
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}



void setup() {
  Serial.begin(9600);
  int a;
  int e;
  for (int m=1; m<13; m++) {
   for(int d=1; d<32; d++) {
    prsunrise(d, m);
   }
  }
}

void loop() {

}
