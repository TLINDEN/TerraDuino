

int schalter[] = { 51, 49, 47, 45, 43, 41 };
int leds[]     = { 50, 48, 46, 44, 42, 40 };
int relais[]   = { 36, 34, 32, 30, 28, 26 };
int state[]    = { 1, 1, 1, 1, 1, 1 };
int mode = 0;
int strom;
  
void setup() {
 Serial.begin(9600);
 for (int i = 0; i < 6; i++) {
  pinMode(schalter[i], INPUT);
  pinMode(leds[i], OUTPUT);
  pinMode(relais[i], OUTPUT);
  mode = digitalRead(schalter[i]);
  state[i] = mode;
  strom = mode == 1 ? 0 : 1;
  digitalWrite(leds[i], mode);
  digitalWrite(relais[i], strom);
 }
}

void loop() {
  for (int i = 0; i < 6; i++) {
    mode = digitalRead(schalter[i]);
    if(mode != state[i]) {
      strom = mode == 1 ? 0 : 1;
      digitalWrite(leds[i], mode);
      digitalWrite(relais[i], strom);
      pr(i, mode);
      state[i] = mode;
    }
  }
  delay(50);
}

void pr(int id, int mode) {
 Serial.print("Schalter ");
 Serial.print(id);
 Serial.print(" Mode: ");
 Serial.print(mode);
 Serial.println(); 
}
