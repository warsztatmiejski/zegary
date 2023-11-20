#include <Wire.h>
#include <ArduinoLowPower.h>

#define PIN_ALARM 3
#define PIN_A 4
#define PIN_B 5
#define PIN_BTN 6
#define PIN_BTN_GND 7
#define PIN_BTN_TURN_ON 8
#define PIN_LED 13

#define TICK 250L
#define MINUTE ((60L * 1000L) / TICK)

static bool polarity = false;
static int16_t slow_counter = 0;

byte rtc_read(byte addr) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((addr << 1) | 0x01);
  byte data = Wire.read();
  Wire.endTransmission();
  return data;
}

void rtc_write(byte addr, byte data) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(addr << 1);
  Wire.write(data);
  Wire.endTransmission();
}

void setup() {
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  pinMode(PIN_ALARM, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BTN_TURN_ON, INPUT_PULLUP);
  pinMode(PIN_BTN_GND, OUTPUT);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_BTN_GND, LOW);

  LowPower.attachInterruptWakeup(PIN_ALARM, alarm, FALLING);
  LowPower.attachInterruptWakeup(PIN_BTN, change_time, FALLING);

  Wire.begin();
  if (rtc_read(0x0F) >> 7) { // When the microcontroller was turned off but RTC has working.
    // TODO restore time
  } else {  // When RTC lost the power.
    { // set time to 12:00:00AM (12h mode because of clock face).
      rtc_write(0x00, 0b00000000); // 00 seconds.
      rtc_write(0x01, 0b00000000); // 00 minutes.
      rtc_write(0x02, 0b01000000); // 12AM hour.
    }
    { // set alarm1 (as last time) to 12:00:00AM
      rtc_write(0x07, 0b00000000); // 00 seconds.
      rtc_write(0x08, 0b00000000); // 00 minutes.
      rtc_write(0x09, 0b01000000); // 12AM hour.
    }
    { // set alarm2 to 12:01PM
      rtc_write(0x07, 0b00000000); // 00 seconds.
      rtc_write(0x08, 0b00000000); // 00 minutes.
      rtc_write(0x09, 0b01000000); // 12AM hour.
    }

    rtc_write(0x0E, 0b00000110); // Set INT pin + set alarm2.
    //rtc_write(0x0F, 0b00000000); // Reset oscillator stop flag.
  }
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

int rtc_format_to_int(byte val) {
  return val + (val >> 4) * 60;
}

void loop() {
  LowPower.sleep();
}

void alarm() {
  tick();
  int minutes = rtc_format_to_int(rtc_read(0x01));
  int alarm_minutes = (minutes + 1) % 60;
  rct_write(0x07, 0));
  rct_write(0x0B, int_to_rtc_format(alarm_minutes));
  turn_off();
}

void change_time() {
  while (digitalRead(PIN_BTN) == LOW) {
    tick();
    int minutes = rtc_format_to_int(rtc_read(0x01));
    int change_minutes = (minutes + 1) % 60;
    rct_write(0x00, 0));
    rct_write(0x01, int_to_rtc_format(change_minutes));
    delayMicroseconds(1000L * TICK);
  }
  turn_off();
}
