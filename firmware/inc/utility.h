#ifndef UTILITY_H
#define UTILITY_H

#include "pico/stdlib.h"
#include "./definitions.h"

void gpio_toggle(uint pin) {
  gpio_put(pin, !gpio_get(pin));
}

/* [CRC16 GENERATION] ========================================================= */

/* default CRC type: MODBUS */
#if !defined(CRC16_POLYNOMIAL_MIRRORED) || !defined(CRC16_INITIAL)
  #define CRC16_POLYNOMIAL_MIRRORED (0xA001)
  #define CRC16_INITIAL (0xFFFF)
#endif

/**
 * @brief Calculate a CRC16-checksum from the given data.
 * Default Polynomial is the MODBUS. To change, define
 * \c CRC16_POLYNOMIAL_MIRRORED , which is the polynomial
 * but simply mirrored (MSB -> LSB, LSB -> MSB, etc.),
 * and \c CRC16_INITIAL .
 * 
 * @param data Pointer referencing a data-set
 * @param length 
 * @return Returns the calculated checksum (uint16_t) 
 */
uint16_t calculateCRC16(const char* data, uint8_t length) {
  uint16_t checksum = CRC16_INITIAL;

  /* [DATA STEPPER]  ------------------------- */
  for (uint8_t i = 0; i < length; i++) {
    /* (1) XOR the checksum with the data
     */
    checksum = (uint16_t)data[i] ^ checksum;

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
      if(carry_out) {
        checksum ^= CRC16_POLYNOMIAL_MIRRORED;
      }
    }
  }
  return checksum;
}

#endif