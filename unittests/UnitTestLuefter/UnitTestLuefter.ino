int luefter = 24;
int taster  = 51;
int state   = 0;
int cur     = 0;

void setup() {
  pinMode(luefter, OUTPUT);
  pinMode(taster, INPUT);
  state = digitalRead(taster);
}

void loop() {
  cur = digitalRead(taster);
  if(cur != state) {
    state = cur;
    if(cur) {
     digitalWrite(luefter, LOW);
    }
    else {
     digitalWrite(luefter, HIGH);
    }
  }
  delay(100);
}
