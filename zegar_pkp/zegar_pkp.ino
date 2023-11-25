#include "rtc.h"
#include <Wire.h>
#include <LowPower.h>

#define PIN_ALARM 2
#define PIN_A 4
#define PIN_B 5
#define PIN_BTN 3
#define PIN_BTN_TURN_ON 6
#define PIN_LED 13

#define TICK 250L
#define MINUTE ((60L * 1000L) / TICK)
#define CLOCK_DEBUG 1

static bool polarity = false;
static int16_t slow_counter = 0; 


void setup() {
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  pinMode(PIN_ALARM, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BTN_TURN_ON, INPUT_PULLUP);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(PIN_LED, LOW);

  attachInterrupt(digitalPinToInterrupt(PIN_ALARM), alarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), change_time, FALLING);

  Rtc.begin();
  Rtc.reset();

#if CLOCK_DEBUG
  Serial.begin(19200);
  Serial.println("INIT");
#endif
  
  //if (rtc_read(0x0F) >> 7) { // When the microcontroller was turned off but RTC has working.
  //  // TODO restore time
  //} else {  // When RTC lost the power.
  //}
}

void tick() {
  if (digitalRead(PIN_BTN_TURN_ON) == LOW) {
    return;
  }
  if (polarity) {
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, HIGH);
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_A, HIGH);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_LED, LOW);
  }
  polarity = !polarity;
}

void turn_off() {
  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(PIN_LED, LOW); 
}

void loop() {
#if CLOCK_DEBUG
  byte seconds = Rtc.get_seconds();
  byte minutes = Rtc.get_minutes();
  byte hours = Rtc.get_hours();
  Serial.print("time: ");
  if (hours < 10) Serial.print('0');
  Serial.print(hours);
  Serial.print(':');
  if (minutes < 10) Serial.print('0');
  Serial.print(minutes);
  Serial.print(':');
  if (seconds < 10) Serial.print('0');
  Serial.print(seconds);
  Serial.print('\n');
  delay(1000L);
#else
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
#endif
  //change_time();
}

void alarm() {
  #if CLOCK_DEBUG
  Serial.println("ALARM");
  #endif
  tick();
  //int minutes = rtc_format_to_int(rtc_read(0x01));
  //int alarm_minutes = (minutes + 1) % 60;
  //rtc_write(0x07, 0));
  //rtc_write(0x0B, int_to_rtc_format(alarm_minutes));
  turn_off();
}

void change_time() {
  while (digitalRead(PIN_BTN) == LOW) {
    tick();
    //int minutes = rtc_format_to_int(rtc_read(0x01));
    //int change_minutes = (minutes + 1) % 60;
    //rtc_write(0x00, 0);
    //rtc_write(0x01, int_to_rtc_format(change_minutes));
    for(int tick = 0; tick < 1000L; tick++) {
      delayMicroseconds(TICK);
    }
  }
  turn_off();
}
