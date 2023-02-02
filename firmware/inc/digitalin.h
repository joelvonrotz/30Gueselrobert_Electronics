#ifndef DIGITALIN_H
#define DIGITALIN_H

#include "pico/stdlib.h"

class DigitalIn
{
private:
  uint pin;

public:
  DigitalIn(uint pin);
  bool read(void);
  void attach_interrupt(uint32_t event_mask, gpio_irq_callback_t callback, bool enabled = true);
  void attach_interrupt(uint32_t event_mask, bool enabled = true);

  bool operator=(const DigitalIn &digitalIn);
};

#endif /* DIGITALIN_H */
