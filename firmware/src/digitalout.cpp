#include "digitalout.h"

DigitalOut::DigitalOut(uint pin) : pin(pin)
{
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
}

void DigitalOut::write(bool value)
{
  gpio_put(this->pin, value);
}

bool DigitalOut::read(void)
{
  return gpio_get(this->pin);
}