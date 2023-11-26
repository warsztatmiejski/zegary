#pragma once

class Time {
  public:
  Time(byte minutes, byte hours) : _minutes(minutes), _hours(hours) {}
  inline byte minutes() { return _minutes; }
  inline byte hours() { return _hours; }
  
  inline int get_total_minutes() {
    return (int)_minutes + ((int)_hours % 12) * 60;
  }
  
  inline void increment_minute() {
    _minutes += 1;
    if (_minutes >= 60) {
      _minutes = 0;
      increment_hour();
    }
  }

  inline void increment_hour() {
    _hours += 1;
    if (_hours > 12) {
      _hours = 1;
    }
  }

  private:
  byte _minutes;
  byte _hours;
};
