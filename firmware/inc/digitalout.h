#ifndef DIGITALOUT_H
#define DIGITALOUT_H

#include "pico/stdlib.h"

class DigitalOut
{
private:
  uint pin;

public:
  DigitalOut(uint pin);
  void write(bool value);
  bool read(void);
};

#endif /* DIGITALOUT_H */
