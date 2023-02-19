#include "robocom.h"

/* [VARIABLEN] ============================================================== */

uint8_t error_checksum[][2] = {
//  MSB   LSB
//  [0]   [1]
  {0xB0, 0x23}, // TIMEOUT
  {0x70, 0xE2}  // INVALID CHECKSUM
};

/* [FUNCTIONS] ============================================================== */
/* [INTERNAL] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void RobotCommunication::init(uint32_t baudrate) {
  uart_init(uart0, baudrate);

  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

#ifdef ENABLE_LOG
  printf("[LOG] Initializing 'uart0', baudrate: '115200'\r\n");
#endif

  clear_function_list();
  clear_buffer();

  /* clear bluffer */
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 0;
  }
}

void RobotCommunication::reset_state_machine(void) {
  buffer_index = 0;
  state = AWAIT_START;
  type = NONE;

  watchdog_cancel(alarm_id);
  alarm_id = -1;

  index.checksum = -1;
  index.command = -1;
  index.data = -1;
}

void RobotCommunication::default_function(uint8_t* value, uint8_t size) {
  printf("[FUNCTION] No Function attached\r\n");
}

bool RobotCommunication::attach(uint8_t command, robocom_callback_t callback) {
  if (command >= FUNCTION_LIST_SIZE) {
#ifdef ENABLE_LOG
    printf("[LOG] cancelled -> command >= FUNCTION_LIST_SIZE (%u)\r\n",
           FUNCTION_LIST_SIZE);
#endif
    return false;
  }
#ifdef ENABLE_LOG
  if (function_list[command] == default_function) {
    printf("[LOG] Attaching function at command'%u'\r\n", command);
  } else {
    printf("[LOG] Overwriting function at command'%u'\r\n", command);
  }
#endif
  function_list[command] = callback;
  return true;
}

void RobotCommunication::run(void) {
  /*  NOTE   UART
   * When used with UART, removed this chunk and place the function
   * 'uart_getc(...)' further down.
   */
  // int32_t value = getchar_timeout_us(1);  // this one here

  /*  NOTE   UART
   * 'value != PICO_ERROR_TIMEOUT' needs to be replaced with
   * 'uart_is_readable(...)' when RS232/UART is used.
   */
  // if (value != PICO_ERROR_TIMEOUT) {
  if (uart_is_readable(uart0)) {
    /*  NOTE   UART
     * Insert 'int32_t value = uart_getc(...)' below when RS232/UART
     * is used.
     */
    volatile int32_t value = uart_getc(uart0);

#ifdef ENABLE_LOG
    if (buffer_index == 0) {
      printf("[LOG] Collecting Package\r\n");
      printf("[LOG] Starting Watchdog!\r\n");
    }
#endif

    // add the read character to the buffer
    buffer[buffer_index++] = value;

#ifdef ENABLE_LOG
    printf("-> [%u]: <%02X>\r\n", buffer_index - 1, value);
#endif

    switch (state) {
      /* This is the idle state!
       * Await the starting symbol
       */
      case AWAIT_START: {
        if ((value == RESPONSE) || (value == REQUEST) || (value == ERROR)) {
          alarm_id = watchdog_start(this);
        }

        /* [Response, Request] ------------------- */
        if ((value == RESPONSE) || (value == REQUEST)) {
          type = (robocom_package_type_t)value;
          state = READ_SIZE;

          /* (0)    (1)   (2)      (3)   ...(n-1)
           * [Start][Size][Command][Data]...[CRC16]
           */
          index.command = 2;
          index.data = 3;

        }
        /* [Error] ------------------------------- */
        else if (value == ERROR) {
          type = (robocom_package_type_t)value;
          state = READ_DATA;
          data_length = 3;  // one byte

          /* (0)    (1)        (2)
           * [Start][ErrorCode][CRC16]
           */
          index.command = 1;  // the error code
          index.data = -1;    // as the error code has no data, it is not needed
          index.checksum = 2;

        } else {
          state = RESET;
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
        data_length = value + 3;  // + cmd & crc16
        state = READ_DATA;

        index.checksum = data_length;
        break;
      }
      /* Read the data from the input and reduce the data length
       * until the length variable is 'used up'.
       */
      case READ_DATA: {
        if ((--data_length) == 0) {
          watchdog_cancel(alarm_id);
          state = CHECK_CRC;

#ifdef ENABLE_LOG
          printf("[LOG] RECEIVED DATA \r\n");
          // print whole received package
          printf("[%u] ", buffer_index);
          for (uint8_t i = 0; (i < (buffer_index)); i++) {
            printf("<%02X>", buffer[i]);
          }
          printf("\r\n");
          // print command segment
          printf("-> command: <%02X>\r\n", buffer[index.command]);

          // print data segment
          if (index.data != -1) {
            printf("-> data: ");
            for (uint8_t i = index.data; (i < (index.checksum)); i++) {
              printf("<%02X>", buffer[i]);
            }
            printf("\r\n");
          }

          // print checksum segment
          printf("-> checksum: ");
          for (uint8_t i = index.checksum; i < (buffer_index); i++) {
            printf("<%02X>", buffer[i]);
          }
#endif
        }
        break;
      }
      /* This state should never be expected, as only the given
       * 'state_s'-states should be evaluated.
       */
      default: {
#ifdef ENABLE_LOG
        printf(
            "[LOG] In 'default' of the state machine."
            "That shouldn't happen! \r\n");
#endif
        break;
      }
    }
  }

  if (state == RESET) {
    reset_state_machine();

  } else if (state == CHECK_CRC) {
    /*  TODO : Actually check for CRC */
    uint16_t calc_checksum = calculateCRC16(buffer, buffer_index - 2);
    uint16_t pkg_checksum = (((uint16_t)buffer[index.checksum]) << 8) +
                            (((uint16_t)buffer[index.checksum + 1]));

    if(calc_checksum != pkg_checksum) {
      send_error(ERROR_INVALID_CHECKSUM);
    } else {
      uint8_t command = buffer[index.command];

      switch (type) {
        case REQUEST: {
          if (command < FUNCTION_LIST_SIZE) {
            function_list[command](&buffer[index.data],
                                  index.checksum - index.data);
          }
          break;
        }
        case RESPONSE:
        case ERROR:
        default:
          break;
      }

    }
    state = RESET;
  }
}

/* TODO */
void RobotCommunication::send_request(uint8_t command, uint8_t size,
                                      uint8_t* data) {
  uint8_t total_size = size + 5;
  uint8_t send_buffer[total_size];  // data + start + size + cmd + crc16
  // (1) Build Send Buffer
  send_buffer[0] = '?';
  send_buffer[1] = size;
  send_buffer[2] = command;
  for (uint8_t i = 0; i < size; i++) {
    send_buffer[i + 3] = data[i];
  }
  // (2) Calculate Checksum
  uint16_t checksum = calculateCRC16(send_buffer, size + 3);
  send_buffer[size + 3] = (checksum >> 8) & 0x00FF;
  send_buffer[size + 4] = checksum & 0x00FF;

// (3) Send Data
#ifdef ENABLE_LOG
  printf("[LOG] Sending Request 0x%02X\r\n", command);
  printf("-> ");
#endif
  for (uint8_t i = 0; i < total_size; i++) {
#ifdef ENABLE_LOG
    printf("<%02X>", send_buffer[i]);
#endif
    send_data(send_buffer[i]);
  }
#ifdef ENABLE_LOG
  printf("\r\n");
#endif
}

/* TODO */
void RobotCommunication::send_error(uint8_t error) {
  uint8_t total_size = 4;
  uint8_t send_buffer[total_size];  // data + start + size + cmd + crc16
  // (1) Build Send Buffer
  send_buffer[0] = 'x';
  send_buffer[1] = error;

  // (2) Get Precalculated Checksum
  send_buffer[2] = error_checksum[error][0];
  send_buffer[3] = error_checksum[error][1];

  // (3) Send Data
#ifdef ENABLE_LOG
  printf("[LOG] Sending Error 0x%02X\r\n", error);
  printf("-> ");
#endif
  for (uint8_t i = 0; i < total_size; i++) {
#ifdef ENABLE_LOG
    printf("<%02X>", send_buffer[i]);
#endif
    send_data(send_buffer[i]);
  }

#ifdef ENABLE_LOG
  printf("\r\n");
#endif
}

/* TODO */
void RobotCommunication::send_response(uint8_t command, uint8_t size,
                                       uint8_t* data) {
  uint8_t total_size = size + 5;
  uint8_t send_buffer[total_size];  // data + start + size + cmd + crc16
  // (1) Build Send Buffer
  send_buffer[0] = '!';
  send_buffer[1] = size;
  send_buffer[2] = command;
  for (uint8_t i = 0; i < size; i++) {
    send_buffer[i + 3] = data[i];
  }
  // (2) Calculate Checksum
  uint16_t checksum = calculateCRC16(send_buffer, size + 3);
  send_buffer[size + 3] = (checksum >> 8) & 0x00FF;
  send_buffer[size + 4] = checksum & 0x00FF;

  // (3) Send Data
#ifdef ENABLE_LOG
  printf("[LOG] Sending Response 0x%02X\r\n", command);
  printf("-> ");
#endif
  for (uint8_t i = 0; i < total_size; i++) {
#ifdef ENABLE_LOG
    printf("<%02X>", send_buffer[i]);
#endif
    send_data(send_buffer[i]);
  }
#ifdef ENABLE_LOG
  printf("\r\n");
#endif
}

void RobotCommunication::send_data(uint8_t byte) { uart_putc(uart0, byte); }

/* [INTERNAL]
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void RobotCommunication::clear_function_list(void) {
  for (uint8_t i = 0; i < FUNCTION_LIST_SIZE; i++) {
    function_list[i] = default_function;
  }
}

void RobotCommunication::clear_buffer(void) {}

alarm_id_t RobotCommunication::watchdog_start(RobotCommunication* sender) {

  return add_alarm_in_ms(WATCHDOG_TIMING_MS, watchdog_callback, (void*)sender,
                         false);
}

void RobotCommunication::watchdog_cancel(alarm_id_t alarm_id) {
  if (alarm_id >= 0) {
    #ifdef ENABLE_LOG
      printf("[LOG] Watchdog stopped!\r\n");
    #endif
    cancel_alarm(alarm_id);
    alarm_id = -1;
  }
}

int64_t RobotCommunication::watchdog_callback(alarm_id_t id, void* user_data) {
#ifdef ENABLE_LOG
  printf("[LOG] Watchdog timer ended, callback executed!\r\n");
#endif
  RobotCommunication* object = (RobotCommunication*)user_data;
  object->send_error(ERROR_TIMEOUT);
  object->state = RESET;
  return 0;
}
