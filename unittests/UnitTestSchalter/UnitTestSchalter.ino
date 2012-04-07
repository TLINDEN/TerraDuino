

int schalter[] = { 51, 49, 47, 45, 43, 41, 19 };
int leds[]     = { 50, 48, 46, 44, 42, 40, 18 };
int state[]    = { 1, 1, 1, 1, 1, 1, 1 };
int mode = 0;
  
void setup() {
 Serial.begin(9600);
 for (int i = 0; i < 7; i++) {
  pinMode(schalter[i], INPUT);
  pinMode(leds[i], OUTPUT);
  digitalWrite(leds[i], HIGH);
  delay(1000);
 }
}

void loop() {
  for (int i = 0; i < 7; i++) {
    mode = digitalRead(schalter[i]);
    if(mode != state[i]) {
      digitalWrite(leds[i], mode);
      pr(i, mode);
      state[i] = mode;
    }
  }
  delay(100);
}

void pr(int id, int mode) {
 Serial.print("Schalter ");
 Serial.print(id);
 Serial.print(" Mode: ");
 Serial.print(mode);
 Serial.println(); 
}
