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
#define CLOCK_DEBUG 1


enum class Event {WAIT=0, ALARM=1, CHANGE_TIME=2, SYNC=3};

static bool polarity = false;
static Event event = Event::WAIT;

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
  if (Rtc.get_status() & (1 << RTC_STATUS_FLAG_OSF)) { // When RTC lost the power.
    Rtc.reset();
    event = Event::WAIT;
  } else { // When the microcontroller was turned off but RTC was working.
    Rtc.clear_alarm2();
    event = Event::SYNC;
  }

#if CLOCK_DEBUG
  Serial.begin(19200);
  while(!Serial);
  Serial.println("INIT");
#endif
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

#if CLOCK_DEBUG
void print_time(byte hours, byte minutes, byte seconds) {
  if (hours < 10) Serial.print('0');
  Serial.print(hours);
  Serial.print(':');
  if (minutes < 10) Serial.print('0');
  Serial.print(minutes);
  Serial.print(':');
  if (seconds < 10) Serial.print('0');
  Serial.print(seconds);
}

void print_byte(byte val) {
  String a(8);
  for(byte i = 0; i < 8; i++) {
    a[7-i] = val & 1 ? '1' : '0';
    val = val >> 1;
  }
  Serial.print(a);
}
#endif

inline void inc_time(byte* minutes, byte* hours) {
    *minutes += 1;
    if (*minutes >= 60) {
      *minutes = 0;
      *hours += 1;
    }
    if (*hours > 12) {
      *hours = 1;
    }
}

void manage_event() {
  switch(event) {
    case Event::ALARM: {
      tick();
      delayMicroseconds(100L); // wait some time to move the motor.
      turn_off();
      Rtc.clear_alarm2();
      // save last viewed time
      byte seconds = Rtc.get_seconds();
      byte minutes = Rtc.get_minutes();
      byte hours = Rtc.get_hours();
      Rtc.set_alarm1_seconds(seconds);
      Rtc.set_alarm1_minutes(minutes);
      Rtc.set_alarm1_hours(hours);
      event = Event::WAIT;
    } break;
    case Event::CHANGE_TIME: {
      event = Event::WAIT;
      while (digitalRead(PIN_BTN) == LOW) {
        tick();
        byte minutes = Rtc.get_minutes();
        byte hours = Rtc.get_hours();
        inc_time(&minutes, &hours);
        Rtc.set_minutes(minutes);
        Rtc.set_hours(hours);
        Rtc.set_alarm1_minutes(minutes);
        Rtc.set_alarm1_hours(hours);
        for(int t = 0; t < 1000L; t++) {
          delayMicroseconds(TICK);
        }
      }
      turn_off();
      Rtc.clear_alarm2();
    } break;
    case Event::SYNC: {
      byte last_set_minutes = Rtc.get_alarm1_minutes();
      byte last_set_hours = Rtc.get_alarm1_hours();
      int last_set_total_minutes = (int)last_set_minutes + ((int)last_set_hours % 12) * 60;

      byte current_minutes = Rtc.get_minutes();
      byte current_hours = Rtc.get_hours();
      int current_total_minutes = (int)current_minutes + ((int)current_hours % 12) * 60;

      int lack_minutes;
      if (last_set_total_minutes <= current_total_minutes) {
        lack_minutes = current_total_minutes - last_set_total_minutes;
      } else {
        lack_minutes = current_total_minutes + 12 * 60 - last_set_total_minutes;
      }

      for (int i = 0; i < lack_minutes; i++) {
        tick();
        inc_time(&last_set_minutes, &last_set_hours);
        Rtc.set_alarm1_minutes(last_set_minutes);
        Rtc.set_alarm1_hours(last_set_hours);
        for(int t = 0; t < 1000L; t++) {
          delayMicroseconds(TICK);
        }
      }
      turn_off();
      Rtc.clear_alarm2();
      event = Event::WAIT;
    } break;
    default: {
      event = Event::WAIT;
    } break;
  }
}

void loop() {
#if CLOCK_DEBUG
static unsigned int iii = 0;
static unsigned char iiii = 0;
static Event prev_event = event;
if (event != prev_event || (iii++ == 0 && (iiii++ & 0b111) == 0)) {
  prev_event = event;
  Serial.print("event: ");
  switch(event) {
    case Event::WAIT:
      Serial.print("WAIT       ");
      break;
    case Event::ALARM:
      Serial.print("ALARM      ");
      break;
    case Event::CHANGE_TIME:
      Serial.print("CHANGE_TIME");
      break;
    case Event::SYNC:
      Serial.print("SYNC       ");
      break;
    default:
      Serial.print("???        ");
      break;
  }
  Serial.print(" time: ");
  print_time(Rtc.get_hours(), Rtc.get_minutes(), Rtc.get_seconds());
  Serial.print(" alarm1: ");
  print_time(Rtc.get_alarm1_hours(), Rtc.get_alarm1_minutes(), Rtc.get_alarm1_seconds());
  Serial.print(" status: ");
  print_byte(Rtc.get_status());
  Serial.print('\n');
  delayMicroseconds(10000L);
}
#else
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
#endif
  manage_event();
}

inline void int_change_event(Event e) {
  if (event >= e) {
    return;
  }
  event = e;
}

void alarm() {
  int_change_event(Event::ALARM);
}

void change_time() {
  int_change_event(Event::CHANGE_TIME);
}
