

#include <Task.h>
#include <TaskScheduler.h>

int taststate  = 0;
int inputstate = 0;

int taster     = 51;
int tastled    = 50;

int blinkled   = 27;
int blinkdelay = 300;



// Timed task to blink a LED.
class Blinker : public TimedTask {
public:
    // Create a new blinker for the specified pin and rate.
    Blinker(uint8_t _pin, uint32_t _rate);
    virtual void run(uint32_t now);
private:
    uint8_t pin;      // LED pin.
    uint32_t rate;    // Blink rate.
    bool on;          // Current state of the LED.
};

Blinker::Blinker(uint8_t _pin, uint32_t _rate)
: TimedTask(millis()),
  pin(_pin),
  rate(_rate),
  on(false)
{
    pinMode(pin, OUTPUT);     // Set pin for output.
}

void Blinker::run(uint32_t now) {
    // If the LED is on, turn it off and remember the state.
    if (on) {
        digitalWrite(pin, LOW);
        on = false;
    // If the LED is off, turn it on and remember the state.
    } else {
        if(inputstate || taststate) {
           digitalWrite(pin, HIGH);
        }
        on = true;
    }
    // Run again in the required number of milliseconds.
    incRunTime(rate);
}

// Task to echo serial input.
class Echoer : public Task {
public:
    Echoer();
    virtual void run(uint32_t now);
    virtual bool canRun(uint32_t now);
};

Echoer::Echoer()
: Task()
{
    Serial.begin(9600);
    Serial.println("If switch is OFF, enter 1 to turn on the blink and 0 to turn it off");
}

bool Echoer::canRun(uint32_t now) {
    return Serial.available() > 0;
}

void Echoer::run(uint32_t now) {
    while (Serial.available() > 0) {
        int byte = Serial.read();
        if (byte == '0') {
            Serial.println("LED off");
            inputstate = 0;
        }
        else {
            Serial.println("LED on");
            inputstate = 1;
        }
    }
}




  
class Taster : public Task {
public:
  Taster(uint8_t _pin, uint8_t _ledpin);
  virtual void run(uint32_t now);
  virtual bool canRun(uint32_t now);
private:
  uint8_t pin;
  uint8_t ledpin;
  int on;
};

Taster::Taster(uint8_t _pin, uint8_t _ledpin)
: Task(),
  pin(_pin),
  ledpin(_ledpin),
  on(0)
{
    pinMode(pin, INPUT);
    pinMode(ledpin, OUTPUT);
}

bool Taster::canRun(uint32_t now) {
    return true;
}

void Taster::run(uint32_t now) {
  on = digitalRead(pin); 
  if (on) {
    digitalWrite(ledpin, HIGH);
    taststate = 1;
  }
  else {
    digitalWrite(ledpin, LOW);
    taststate = 0;
  }
}





void setup() {
}

void loop() {
    // Create the tasks.
    Blinker blinker(blinkled, blinkdelay);
    Echoer  echoer;
    Taster  knopf(taster, tastled);
    
    // Initialise the task list and scheduler.
    Task *tasks[] = { &blinker, &echoer, &knopf };
    TaskScheduler sched(tasks, NUM_TASKS(tasks));
    
    // Run the scheduler - never returns.
    sched.run(); 
}

