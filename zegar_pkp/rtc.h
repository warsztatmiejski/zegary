#pragma once

#include <Wire.h>

#define RTC_STATUS_FLAG_OSF 7
#define RTC_STATUS_FLAG_BSY 2
#define RTC_STATUS_FLAG_A2F 1
#define RTC_STATUS_FLAG_A1F 0

class _Rtc {
  public:
  void begin() {
    Wire.begin();
  }

  void reset() {
    { // set time to 12:00:00AM (12h mode because of clock face).
      _write(TIME_SECONDS_ADDR, _seconds_encode(0));
      _write(TIME_MINUTES_ADDR, _minutes_encode(0));
      _write(TIME_HOURS_ADDR, _hours_encode(12)); // 12AM hour.
    }
    { // set alarm1 (as last time) to 12:00:00AM
      _write(ALARM1_SECONDS_ADDR, _seconds_encode(0));
      _write(ALARM1_MINUTES_ADDR, _minutes_encode(0));
      _write(ALARM1_HOURS_ADDR, _hours_encode(12));
    }
    { // set alarm2 to 12:01PM + ALARM every second
      _write(ALARM2_MINUTES_ADDR, ALARM_FLAG |_minutes_encode(0));
      _write(ALARM2_HOURS_ADDR, ALARM_FLAG | _hours_encode(12));
      _write(ALARM2_DAYS_ADDR, ALARM_FLAG | 1);  
    }

    _write(CONTROL_ADDR, 0b00000110); // Set INT pin + set alarm2.
    _write(STATUS_ADDR, 0b00000000); // Reset oscillator stop flag.
  }

  byte get_seconds() {
    return _seconds_decode(_read(TIME_SECONDS_ADDR));
  }
  
  byte get_minutes() {
    return _minutes_decode(_read(TIME_MINUTES_ADDR));
  }
  
  byte get_hours() {
    return _hours_decode(_read(TIME_HOURS_ADDR));
  }

  byte get_alarm1_seconds() {
    return _seconds_decode(_read(ALARM1_SECONDS_ADDR));
  }
  
  byte get_alarm1_minutes() {
    return _minutes_decode(_read(ALARM1_MINUTES_ADDR));
  }
  
  byte get_alarm1_hours() {
    return _hours_decode(_read(ALARM1_HOURS_ADDR));
  }

  byte get_status() {
    return _read(STATUS_ADDR);
  }

  void set_seconds(byte seconds) {
    _write(TIME_SECONDS_ADDR, _seconds_encode(seconds));
  }

  void set_minutes(byte minutes) {
    _write(TIME_MINUTES_ADDR, _seconds_encode(minutes));
  }

  void set_hours(byte hours) {
    _write(TIME_HOURS_ADDR, _seconds_encode(hours));
  }

  void set_alarm1_seconds(byte seconds) {
    _write(ALARM1_SECONDS_ADDR, _seconds_encode(seconds));
  }

  void set_alarm1_minutes(byte minutes) {
    _write(ALARM1_MINUTES_ADDR, _seconds_encode(minutes));
  }

  void set_alarm1_hours(byte hours) {
    _write(ALARM1_HOURS_ADDR, _seconds_encode(hours));
  }

  void clear_alarm1() {
    _write(STATUS_ADDR, get_status() & ~0b01);
  }

  void clear_alarm2() {
    _write(STATUS_ADDR, get_status() & ~0b10);
  }
  
  private:
  const byte TIME_SECONDS_ADDR = 0x00;
  const byte TIME_MINUTES_ADDR = 0x01;
  const byte TIME_HOURS_ADDR = 0x02;
  const byte ALARM1_SECONDS_ADDR = 0x07;
  const byte ALARM1_MINUTES_ADDR = 0x08;
  const byte ALARM1_HOURS_ADDR = 0x09;
  const byte ALARM2_MINUTES_ADDR = 0x0B;
  const byte ALARM2_HOURS_ADDR = 0x0C;
  const byte ALARM2_DAYS_ADDR = 0x0D;
  const byte CONTROL_ADDR = 0x0E;
  const byte STATUS_ADDR = 0x0F;
  const byte HOUR12_FLAG = 0b01000000;
  const byte ALARM_FLAG = 0b10000000;
  const int RTC_ADDR = 0x68;
  
  byte _read(byte addr) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(addr);
    Wire.endTransmission();
    Wire.requestFrom(RTC_ADDR, 1);
    while (Wire.available()) {
      return Wire.read();
    }
  }
  
  void _write(byte addr, byte data) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
  }

  byte _seconds_decode(byte val) {
    return (val & 0b1111) + ((val >> 4) & 0b111) * 10;
  }

  byte _minutes_decode(byte val) {
    return (val & 0b1111) + ((val >> 4) & 0b111) * 10;
  }

  byte _hours_decode(byte val) {
    return (val & 0b1111) + ((val >> 4) & 0b1) * 10;
  }

  byte _seconds_encode(byte val) {
    return (val % 10) | (((val / 10) % 6) << 4);
  }

  byte _minutes_encode(byte val) {
    return (val % 10) | (((val / 10) % 6) << 4);
  }

  byte _hours_encode(byte val) {
    return HOUR12_FLAG | (val % 10) | (((val / 10) % 2) << 4); 
  }
  
} Rtc;
