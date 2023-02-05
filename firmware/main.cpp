/* [INCLUDES] ================== */
#include <stdio.h>
#include "pico/stdlib.h"

/* [HARDWARE] ------------------ */
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

/* [LOCAL] --------------------- */
#include "utility.h"

/* [DEFINES] ~~~~~~~~~~~~~~~~~~~ */
#define ENABLE_LOG

#define LED_PIN (PICO_DEFAULT_LED_PIN)

/* ============================= */




double angles[3] = {0, 0, 0};

uint8_t buffer_index = 0;
const uint8_t BUFFER_SIZE = 64;
uint8_t buffer[BUFFER_SIZE];

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

serial_state_t serial_state = AWAIT_START;
serial_package_type_t serial_type = NONE;
uint8_t data_length;
uint8_t package_size;


int16_t index_command = -1;
int16_t index_checksum = -1;
int16_t index_data = -1;

int main(void)
{
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  // gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  // gpio_set_irq_enabled(22, GPIO_IRQ_EDGE_RISE || GPIO_IRQ_EDGE_FALL, true);

  /* clear bluffer */
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 0;
  }

  while (true)
  {
    /*  NOTE   UART 
     * When used with UART, removed this chunk and place the function
     * 'uart_getc(...)' further down.
     */
    int32_t value = getchar_timeout_us(1);

    if(serial_state == RESET) {
      buffer_index = 0;
      serial_state = AWAIT_START;

      index_checksum = -1;
      index_command = -1;
      index_data = -1;

      #ifdef ENABLE_LOG
      printf("[LOG] Reset done -> Await Start Symbol\n\r");
      #endif
    } 
    else if (serial_state == CHECK_CRC) {
      #ifdef ENABLE_LOG
      for (uint8_t i = 0; (i < (buffer_index)); i++) {
        printf("<%02i>", buffer[i]);
      }
      printf(" - [%u]\n\r[COMMAND]: <%02i>\n\r[DATA]: ",
              buffer_index,
              buffer[index_command]);
      for (uint8_t i = index_data;
           (i < (index_checksum)) && (index_data != -1);
           i++) {
        printf("<%02i>", buffer[i]);
      }
      printf("\n\r[CHECKSUM]: ");
      for (uint8_t i = index_checksum; i < (buffer_index); i++) {
        printf("<%02i>", buffer[i]);
      }
      printf("\n\r");
      #endif

      serial_state = RESET;
    }

    /*  NOTE   UART 
     * 'value != PICO_ERROR_TIMEOUT' needs to be replaced with
     * 'uart_is_readable(...)' when RS232/UART is used.
     */
    if(value != PICO_ERROR_TIMEOUT) {
      /*  NOTE   UART 
       * Insert 'int32_t value = uart_getc(...)' below when RS232/UART
       * is used.
       */
      
      // add the read character to the buffer
      buffer[buffer_index++] = value;

      #ifdef ENABLE_LOG
      // toggle LED when a character has been received
      gpio_toggle(LED_PIN);
      printf("data[%u] received -> <%02i>\n\r",
              buffer_index - 1,
              value);
      #endif

      switch (serial_state) {
        /* This is the idle state!
         * Await the starting symbol
         */
        case AWAIT_START: {
          /* [Response, Request] ------------------- */
          if((value == RESPONSE) || (value == REQUEST)) {
            serial_type = (serial_package_type_t)value;
            serial_state = READ_SIZE;

            /* (0)    (1)   (2)      (3)   ...(n-1)    
             * [Start][Size][Command][Data]...[CRC16]
             */
            index_command = 2;
            index_data = 3;

            #ifdef ENABLE_LOG
            printf("[LOG] Package Type: %s\n\r",
                   (value == RESPONSE) ? "Response" : "Request");
            #endif
          }
          /* [Error] ------------------------------- */
          else if(value == ERROR) {

            serial_type = (serial_package_type_t)value;
            serial_state = READ_DATA;
            data_length = 3; // one byte

            /* (0)    (1)        (2)
             * [Start][ErrorCode][CRC16]
             */
            index_command = 1; // the error code
            index_data = -1; // as the error code has no data, it is not needed
            index_checksum = 2;

            #ifdef ENABLE_LOG
            printf("[LOG] Package Type: Error\n\r");
            printf("[LOG] Index Command: %i\n\r", index_command);
            printf("[LOG] Index Data: %i\n\r", index_data);
            printf("[LOG] Index Checksum: %i\n\r", index_checksum);
            #endif
          } else {
              #ifdef ENABLE_LOG
              printf("[LOG] Package Type: %s -> Reset\n\r", "None");
              #endif
              serial_state = RESET;
          }
          break;
        }
        /* Reads the size byte and applies the read value and the
         * additional sizes for the checksum and command to the
         * data_length variable.
         * 
         * Additionally assigns the index for the checksum for the
         * later data evaluation.
         */
        case READ_SIZE: {
          data_length = value + 3; // + cmd & crc16
          serial_state = READ_DATA;
          
          index_checksum = data_length;
          #ifdef ENABLE_LOG
          printf("[LOG] Package Size: %u\n\r", data_length);
          #endif
          break;
        }
        /* Read the data from the input and reduce the data length
         * until the length variable is 'used up'.
         */
        case READ_DATA: {
          if((--data_length) == 0){
            #ifdef ENABLE_LOG
            printf("[LOG] Finished Data Reading -> Check CRC16\n\r");
            #endif
            serial_state = CHECK_CRC;
          }
          break;
        }
        /* This state should never be expected, as only the given
         * 'serial_state_s'-states should be evaluated.
         */
        default: {
          #ifdef ENABLE_LOG
          printf("[LOG] In 'default' of the state machine." 
                 "That shouldn't happen! \n\r");
          #endif
          break;
        }
      }
    }
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