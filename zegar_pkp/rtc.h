#pragma once

#include <Wire.h>

class _Rtc {
  public:
  void begin() {
    Wire.begin();
  }

  void reset() {
    { // set time to 12:00:00AM (12h mode because of clock face).
      _write(TIME_SECONDS_ADDR, 0b00000000); // 00 seconds.
      _write(TIME_MINUTES_ADDR, 0b00000000); // 00 minutes.
      _write(TIME_HOURS_ADDR, 0b01000000); // 12AM hour.
    }
    { // set alarm1 (as last time) to 12:00:00AM
      _write(ALARM1_SECONDS_ADDR, 0b00000000); // 00 seconds.
      _write(ALARM1_MINUTES_ADDR, 0b00000000); // 00 minutes.
      _write(ALARM2_HOURS_ADDR, 0b01000000); // 12AM hour.
    }
    { // set alarm2 to 12:01PM + ALARM every second
      _write(ALARM2_MINUTES_ADDR, 0b10000000); // 00 minutes.
      _write(ALARM2_HOURS_ADDR, 0b11000000); // 12AM hour. 
      _write(ALARM2_DAYS_ADDR, 0b100000001); // 1 day?    
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
    return (val % 10) + (((val / 10) % 6) << 4);
  }

  byte _minutes_encode(byte val) {
    return (val % 10) + (((val / 10) % 6) << 4);
  }

  byte _hours_encode(byte val) {
    return (val % 10) + (((val / 10) % 2) << 4);
  }
  
} Rtc;
