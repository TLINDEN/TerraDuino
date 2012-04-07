#include <Task.h>
#include <TaskScheduler.h>

// Timed task to blink a LED.
class Blinker : public TimedTask
{
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

void Blinker::run(uint32_t now)
{
    // If the LED is on, turn it off and remember the state.
    if (on) {
        digitalWrite(pin, LOW);
        on = false;
    // If the LED is off, turn it on and remember the state.
    } else {
        digitalWrite(pin, HIGH);
        on = true;
    }
    // Run again in the required number of milliseconds.
    incRunTime(rate);
}

// Task to echo serial input.
class Echoer : public Task
{
public:
    Echoer();
    virtual void run(uint32_t now);
    virtual bool canRun(uint32_t now);
};

Echoer::Echoer()
: Task()
{
    Serial.begin(9600);
}


bool Echoer::canRun(uint32_t now)
{
    return Serial.available() > 0;
}

void Echoer::run(uint32_t now)
{
    while (Serial.available() > 0) {
        int byte = Serial.read();
        Serial.print(byte, BYTE);
        if (byte == '\r') {
            Serial.print('\n', BYTE);
        }
    }
}

void setup()
{
}

// Main program.
void loop()
{
    // Create the tasks.
    Blinker blinker(13, 25);
    Echoer echoer;
    
    // Initialise the task list and scheduler.
    Task *tasks[] = { &blinker, &echoer };
    TaskScheduler sched(tasks, NUM_TASKS(tasks));
    
    // Run the scheduler - never returns.
    sched.run();
}
