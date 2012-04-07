


int leds[]     = { 50, 48, 46, 44, 42, 40, 18 };

  
void setup() {
 for (int i = 0; i < 8; i++) {
  pinMode(leds[i], OUTPUT);
  digitalWrite(leds[i], HIGH);
 }
}

void loop() {
}
