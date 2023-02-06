/* [INCLUDES] ===================================================== */
#include <stdio.h>
#include "pico/stdlib.h"

/* [HARDWARE] ----------------------------------------------------- */
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

/* [LOCAL] -------------------------------------------------------- */
#include "definitions.h"
#include "utility.h"
#include "serial.h"

/* [FUNCTIONS] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* Test  ------- */

void test_set_led(uint8_t* data, uint8_t size) {
  #ifdef ENABLE_LOG
  printf("[LOG] Setting LED\n\r[DATA] ");
  #endif

  for (uint8_t i = 0; i < size; i++) {
    printf("<%u>",data[i]);
  }
  printf("\n\r");
  
  gpio_put(LED_PIN, true);
}

void test_clear_led(uint8_t* data, uint8_t size) {
  #ifdef ENABLE_LOG
  printf("[LOG] Clearing LED\n\r[DATA] ");
  #endif

  for (uint8_t i = 0; i < size; i++) {
    printf("<%u>",data[i]);
  }
  printf("\n\r");

  gpio_put(LED_PIN, false);
}

void test_toggle_led(uint8_t* data, uint8_t size) {
  #ifdef ENABLE_LOG
  printf("[LOG] Toggling LED\n\r[DATA] ");
  #endif

  for (uint8_t i = 0; i < size; i++) {
    printf("<%u>",data[i]);
  }
  printf("\n\r");

  gpio_toggle(LED_PIN);
}

/* [MAIN] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main(void)
{
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  /* TODO  move to serial_init, when used with uart*/
  serial_clear_function_list();

  serial_attach(0x1,test_set_led);
  serial_attach(0x2,test_clear_led);
  serial_attach(0x3,test_toggle_led);
  

  // gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  // gpio_set_irq_enabled(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true);

  while (true) {
    serial_run(); // do one run of the serial process
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