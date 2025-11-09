#include <Arduino.h>

#define LED_PIN1 2
#define LED_PIN2 3

struct Led {
  int pin;
  unsigned long interval;
  unsigned long lastToggle;
  bool state;
};

Led led1 = { LED_PIN1, 0, 0, false };
Led led2 = { LED_PIN2, 0, 0, false };

int selectedLed = LED_PIN1;
int currentInterval = 0;
bool waitingForLed = true;
bool waitingForInterval = false;

void clearSerial() { while (Serial.available() > 0) Serial.read(); }

void ledTask(Led &l) {
  if (l.interval == 0) return;
  unsigned long now = millis();
  if (now - l.lastToggle >= l.interval) {
    l.state = !l.state;
    digitalWrite(l.pin, l.state ? HIGH : LOW);
    l.lastToggle = now;
  }
}

void serialTask() {
  static bool promptShown = false;
  if (!promptShown) {
    if (waitingForLed) Serial.println("Which LED? (2 or 3): ");
    else Serial.println("Enter interval (ms): ");
    promptShown = true;
  }
  if (Serial.available() == 0) return;

  if (waitingForLed) {
    int val = Serial.parseInt();
    clearSerial();
    if (val == LED_PIN1 || val == LED_PIN2) {
      selectedLed = val;
      waitingForLed = false;
      waitingForInterval = true;
      promptShown = false;
    } else {
      Serial.println("Invalid LED. Enter 2 or 3.");
      promptShown = false;
    }
  } else {
    int ms = Serial.parseInt();
    clearSerial();
    if (ms > 0) {
      if (selectedLed == LED_PIN1) { led1.interval = ms; led1.lastToggle = millis(); }
      if (selectedLed == LED_PIN2) { led2.interval = ms; led2.lastToggle = millis(); }
      waitingForLed = true;
      waitingForInterval = false;
      promptShown = false;
    } else {
      Serial.println("Invalid interval. Try again.");
      promptShown = false;
    }
  }
}

typedef void (*TaskFn)();
TaskFn tasks[] = { [](){ ledTask(led1); }, [](){ ledTask(led2); }, serialTask };
const int NUM_TASKS = 3;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  digitalWrite(LED_PIN1, LOW);
  digitalWrite(LED_PIN2, LOW);
}

void loop() {
  for (int i = 0; i < NUM_TASKS; i++) tasks[i]();
}
