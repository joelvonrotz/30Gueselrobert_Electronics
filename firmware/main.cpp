/* [INCLUDES]
 * ========================================================================= */
#include <stdio.h>

#include "pico/stdlib.h"

/* [HARDWARE]
 * ------------------------------------------------------------------------- */
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

/* [LOCAL]
 * ----------------------------------------------------------------------------
 */
#include "definitions.h"
#include "robocom.h"
#include "utility.h"

RobotCommunication robocom = RobotCommunication();

/* [FUNCTIONS]
 * ======================================================================== */
/* Attachable Functions
 * --------------------------------------------------------------- */

void test_set_led(uint8_t* data, uint8_t size) {
#ifdef ENABLE_LOG
  printf("[LOG] Setting LED ~~~~~~~~~~~~\r\n");
  if (size > 0) {
    printf("[DATA] ");
    for (uint8_t i = 0; i < size; i++) {
      printf("<%u>", data[i]);
    }
    printf("\r\n");
  } else {
    printf("- no data...");
  }
#endif

  robocom.send_response(1, 0, NULL);

  gpio_put(LED_PIN, true);
}

void test_clear_led(uint8_t* data, uint8_t size) {
#ifdef ENABLE_LOG
  printf("[LOG] Clearing LED ~~~~~~~~~~~\r\n");
  if (size > 0) {
    printf("[DATA] ");
    for (uint8_t i = 0; i < size; i++) {
      printf("<%u>", data[i]);
    }
    printf("\r\n");
  } else {
    printf("- no data...");
  }
#endif

  robocom.send_response(2, 0, NULL);

  gpio_put(LED_PIN, false);
}

void test_toggle_led(uint8_t* data, uint8_t size) {
#ifdef ENABLE_LOG
  printf("[LOG] Toggling LED ~~~~~~~~~~~\r\n");
  if (size > 0) {
    printf("[DATA] ");
    for (uint8_t i = 0; i < size; i++) {
      printf("<%u>", data[i]);
    }
    printf("\r\n");
  } else {
    printf("Contains no data...\r\n");
  }
#endif

  robocom.send_response(3, 0, NULL);

  gpio_toggle(LED_PIN);
}

/* [MAIN]
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

int main(void) {
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  robocom.init(115200);
  robocom.attach(0x1, test_set_led);
  robocom.attach(0x2, test_clear_led);
  robocom.attach(0x3, test_toggle_led);

  // gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_EDGE_RISE ||
  // GPIO_IRQ_EDGE_FALL, true, &gpio_callback); gpio_set_irq_enabled(22,
  // GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true);

  while (true) {
    robocom.run();
    // printf("hello world");
  }
  return 0;
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