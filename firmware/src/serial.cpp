#include "serial.h"

/* [DEFINES] ------------------------------------------------------ */
#define BUFFER_SIZE (64)
#define FUNCTION_LIST_SIZE (32)

/* [FUNCTION PROTOTYPES] ------------------------------------------ */
static void _serial_default_function(uint8_t*, uint8_t);

/* [VARIABLES] ---------------------------------------------------- */
void (*serial_function_list[FUNCTION_LIST_SIZE])(uint8_t*, uint8_t);

uint8_t buffer_index = 0;
uint8_t buffer[BUFFER_SIZE];

serial_state_t serial_state = AWAIT_START;
serial_package_type_t serial_type = NONE;
uint8_t data_length;
uint8_t package_size;

int16_t index_command = -1;
int16_t index_checksum = -1;
int16_t index_data = -1;

/* [FUNCTIONS] ---------------------------------------------------- */
void serial_init(uint32_t baudrate = 115200) {
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  uart_init(uart0, baudrate);

  #ifdef ENABLE_LOG
  printf("[LOG] Initializing 'uart0', baudrate: '115200'\n\r");
  #endif

  serial_clear_function_list();

  /* clear bluffer */
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 0;
  }
}

static void _serial_default_function(uint8_t* value, uint8_t length) {
  printf("[FUNCTION] No Function attached\n\r");
}

void serial_clear_function_list(void) {
  for (uint8_t i = 0; i < FUNCTION_LIST_SIZE; i++)
  {
    serial_function_list[i] = _serial_default_function;
  }
}

void serial_attach(uint8_t command, void (*callback)(uint8_t*, uint8_t)) {
  #ifdef ENABLE_LOG
  if(serial_function_list[command] == _serial_default_function) {
    printf("[LOG] Overwriting function at '%u'\n\r", command);
  }
  else{
    printf("[LOG] Attaching function at '%u'\n\r", command);
  }
  #endif
  serial_function_list[command] = callback;
}




void serial_run(void) {
  /*  NOTE   UART 
   * When used with UART, removed this chunk and place the function
   * 'uart_getc(...)' further down.
   */
  int32_t value = getchar_timeout_us(1); // this one here

  

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

    /*  TODO : Actually check for CRC */
   
    uint8_t command = buffer[index_command];
    uint8_t* ptr_data = &buffer[index_data];
    uint8_t length = index_checksum - index_data;
    switch (serial_type)
    {
      case REQUEST: {
        if(command < FUNCTION_LIST_SIZE) {
          serial_function_list[command](ptr_data,length);
        }
        break;
      }
      case RESPONSE:
      case ERROR:
      default:
        break;
    }
    

    serial_state = RESET;


    #ifdef ENABLE_LOG
    for (uint8_t i = 0; (i < (buffer_index)); i++) {
      printf("<%02i>", buffer[i]);
    }
    printf(" - [%u]\n\r[COMMAND]: <%02i>\n\r",
            buffer_index,
            buffer[index_command]);
    if(index_data != -1) {
      printf("[DATA]: ");
      for (uint8_t i = index_data; (i < (index_checksum)); i++) {
        printf("<%02i>", buffer[i]);
      }
    }
    printf("\n\r[CHECKSUM]: ");
    for (uint8_t i = index_checksum; i < (buffer_index); i++) {
      printf("<%02i>", buffer[i]);
    }
    printf("\n\r");
    #endif
  }
}