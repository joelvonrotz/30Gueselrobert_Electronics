#include "digitalin.h"

DigitalIn::DigitalIn(uint pin) : pin(pin)
{
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
}

bool DigitalIn::read(void)
{
  return gpio_get(pin);
}

bool DigitalIn::operator=(const DigitalIn &digitalIn)
{
  return gpio_get(digitalIn.pin);
}

/**
 * @brief Attaches GPIO-pin with given `event_mask` to given `callback`
 *
 * @param event_mask `GPIO_IRQ_EDGE_FALL`, `GPIO_IRQ_EDGE_RISE`, `GPIO_IRQ_LEVEL_HIGH`, GPIO_IRQ_LEVEL_LOW
 * @param callback Pointer to desired function (must be of signature `void(* gpio_irq_callback_t) (uint gpio, uint32_t event_mask)`)
 */
void DigitalIn::attach_interrupt(uint32_t event_mask, gpio_irq_callback_t callback, bool enabled = true)
{
  gpio_set_irq_enabled_with_callback(this->pin, event_mask, enabled, callback);
}

/**
 * @brief Attaches GPIO-pin with given `event_mask` to existing callback
 *
 * @param event_mask GPIO_IRQ_EDGE_FALL, GPIO_IRQ_EDGE_RISE, GPIO_IRQ_LEVEL_HIGH, GPIO_IRQ_LEVEL_LOW
 */
void DigitalIn::attach_interrupt(uint32_t event_mask, bool enabled = true)
{
  gpio_set_irq_enabled(this->pin, event_mask, enabled);
}