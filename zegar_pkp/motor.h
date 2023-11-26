#pragma once

#define PIN_A 4
#define PIN_B 5
#define PIN_LED 13

class _Motor {
public:
  void begin() {
    _polarity = false;
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    turn_off();
  }

  void turn_off() {
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_LED, LOW);
  }

  void tick() {
    if (_polarity) {
      digitalWrite(PIN_A, LOW);
      digitalWrite(PIN_B, HIGH);
      digitalWrite(PIN_LED, HIGH);
    } else {
      digitalWrite(PIN_A, HIGH);
      digitalWrite(PIN_B, LOW);
      digitalWrite(PIN_LED, LOW);
    }
    _polarity = !_polarity;
  }

private:
  bool _polarity;
} Motor;
