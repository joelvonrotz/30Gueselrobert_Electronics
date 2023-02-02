/* [INCLUDES] ================== */
#include <stdio.h>
#include "pico/stdlib.h"

/* [HARDWARE] ------------------ */
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

/* [LOCAL] --------------------- */
#include "lib/digitalout.h"
#include "scara.h"
/* ============================= */

DigitalOut led = DigitalOut(PICO_DEFAULT_LED_PIN);

SCARA scara = SCARA(2.0f, 2.0f, 2.0f);
double angles[3] = {0, 0, 0};
char buffer[512];
int main()
{
  stdio_init_all();

  // gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  // gpio_set_irq_enabled(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true);

  while (true)
  {
    led.write(!led.read());
    scara.calculateAngles(-2, 3, angles);
    printf("P = (-2, 3)\n");
    printf("θ1 = %3.3f° | θ2 = %3.3f° | θ3 = %3.3f°\n\n", RAD2DEG(angles[INDEX_θ1]), RAD2DEG(angles[INDEX_θ2]), RAD2DEG(angles[INDEX_θ3]));

    scara.calculateAngles(2, 3, angles);
    printf("P = (2, 3)\n");
    printf("θ1 = %3.3f° | θ2 = %3.3f° | θ3 = %3.3f°\n\n", RAD2DEG(angles[INDEX_θ1]), RAD2DEG(angles[INDEX_θ2]), RAD2DEG(angles[INDEX_θ3]));

    scara.calculateAngles(0, 6, angles);
    printf("P = (0, 6)\n");
    printf("θ1 = %3.3f° | θ2 = %3.3f° | θ3 = %3.3f°\n\n", RAD2DEG(angles[INDEX_θ1]), RAD2DEG(angles[INDEX_θ2]), RAD2DEG(angles[INDEX_θ3]));

    sleep_ms(1000);
  }
}

/* void gpio_callback(uint gpio, uint32_t events)
{
  if (gpio == 22)
  {
    // Do something...
}
if (gpio == 21)
{
  // Do something else...
}
}
*/