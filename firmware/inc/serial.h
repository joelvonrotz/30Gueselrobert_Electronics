#ifndef SERIAL_H // square.h included from main.cpp
#define SERIAL_H // SERIAL_H gets defined here

/* [INCLUDES] ----------------------------------------------------- */
#include <stdio.h>
#include "pico/stdlib.h"
#include "definitions.h"

/* [ENUMERATIONS] ------------------------------------------------- */

/**
 * @brief States for the state machines used in the evaluation of the
 * package.
 */
typedef enum serial_state_s{
  // The idle state of the state machine, awaits the starting symbol
  AWAIT_START,
  // Depending on the package type, this state is used to read the
  // length/size byte and assign it to the data_length variable.
  READ_SIZE,
  // Read the data using the length previously read.
  READ_DATA,
  // Check the CRC16 checksum from the data with the calculated one
  CHECK_CRC,
  // Reset everything so AWAIT_START works
  RESET
} serial_state_t;

/**
 * @brief 
 * 
 */
typedef enum serial_error_s{
  ERROR_NO_START_SYMBOL
} serial_error_t;

/**
 * @brief Starting Symbol Type. Each package type is defined by its
 * starting symbol (and the enum uses the respective ASCII values).
 * 
 * This is done for easier handling in the evaluation process.
 */
typedef enum serial_package_type_s{
  NONE,
  REQUEST = '?',
  RESPONSE = '!',
  ERROR = 'x'
} serial_package_type_t;

/* [FUNCTION-PROTOTYPES] ------------------------------------------ */
void serial_init(uint32_t baudrate);
void serial_clear_function_list(void);
void serial_attach(uint8_t command, void (*callback)(uint8_t*, uint8_t));
void serial_run(void);

#endif // SERIAL_H
