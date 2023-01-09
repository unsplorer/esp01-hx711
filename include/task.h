#include <core_esp8266_features.h>

class Task {
private:
  unsigned long lastRun;
  unsigned int interval;
  void (*_actualTask_)();

public:
  Task(unsigned long lastRun, unsigned int interval, void(func)()) {
    this->lastRun = lastRun;
    this->interval = interval;
    this->_actualTask_ = func;
  }
  void run() {
    this->lastRun = millis();
    this->_actualTask_();
  }
  bool isReady() {
    return ((millis() - this->lastRun) > this->interval) ? true : false;
  }
};