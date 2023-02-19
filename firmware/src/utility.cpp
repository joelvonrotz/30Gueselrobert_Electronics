/* [INCLUDES]
 * ========================================================================= */
#include "utility.h"
#include <stdio.h>
#include "pico/stdlib.h"

/* [FUNCTION IMPLEMENTATIONS]
 * ========================================================= */

void gpio_toggle(uint pin) { gpio_put(pin, !gpio_get(pin)); }

uint16_t calculateCRC16(const uint8_t* data, uint8_t size) {
#ifdef ENABLE_LOG
  printf("[LOG] CALCULATE CRC16\r\n");
  printf("-> size: %u bytes\r\n", size);
  printf("-> data: ");
#endif

  uint16_t checksum = CRC16_INITIAL;

  /* [DATA STEPPER]  ------------------------- */
  for (uint8_t i = 0; i < size; i++) {
    /* (1) XOR the checksum with the data
     */
    checksum = (uint16_t)data[i] ^ checksum;

#ifdef ENABLE_LOG
    printf("<%02X>", data[i]);
#endif

    /* [BIT STEPPER] ------------------------- */
    /* (2) Go through each bit of the checksum, while also
     * bit-shifting it.
     * Important to note is the mirrored polygon!
     */
    for (uint8_t j = 0; j < BITS_IN_BYTES; j++) {
      bool carry_out = checksum & 0x0001;
      checksum >>= 1;

      /* (3) Is the carry_out true, then the polygon is applied.
       * This is a bitwise division using the XOR operation.
       */
      if (carry_out) {
        checksum ^= CRC16_POLYNOMIAL_MIRRORED;
      }
    }
  }

#ifdef ENABLE_LOG
  printf("\r\n-> checksum: <%02X><%02X>\r\n", (checksum >> 8) & 0xFF,
         checksum & 0xFF);
  printf("\r\n");
#endif
  return checksum;
}
