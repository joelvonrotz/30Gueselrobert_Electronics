#ifndef UTILITY_H
#define UTILITY_H

#include "pico/stdlib.h"
#include "definitions.h"

/* [DEFINES] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* default CRC type: MODBUS */
#if !defined(CRC16_POLYNOMIAL_MIRRORED) || !defined(CRC16_INITIAL)
  #define CRC16_POLYNOMIAL_MIRRORED (0xA001)
  #define CRC16_INITIAL (0xFFFF)
#endif


/* [FUNCTION PROTOTYPES] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Inverts the pin's output state (true -> false, false -> true)
 * 
 * @param pin Pin that gets toggled
 */
void gpio_toggle(uint pin);

/**
 * @brief Calculate a CRC16-checksum from the given data.
 * Default Polynomial is the MODBUS. To change, define
 * \c CRC16_POLYNOMIAL_MIRRORED , which is the polynomial
 * but simply mirrored (MSB -> LSB, LSB -> MSB, etc.),
 * and \c CRC16_INITIAL .
 * 
 * @param data Pointer referencing the dataset
 * @param size Size of the dataset
 * @return Returns the calculated checksum (uint16_t)
 */
uint16_t calculateCRC16(const uint8_t* data, uint8_t size);

#endif