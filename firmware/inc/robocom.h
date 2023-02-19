#ifndef ROBOCOM_H  // square.h included from main.cpp
#define ROBOCOM_H  // ROBOCOM_H gets defined here

/* [INCLUDES] =============================================================== */
#include <stdio.h>

#include "definitions.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "utility.h"

/* [DEFINITIONS] ============================================================ */
#ifndef FUNCTION_LIST_SIZE
#define FUNCTION_LIST_SIZE (32)
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE (64)
#endif

#ifndef WATCHDOG_TIMING_MS
#define WATCHDOG_TIMING_MS (100)
#endif

/* [ERROR CODES] */
#define ERROR_TIMEOUT (0x0)
#define ERROR_INVALID_CHECKSUM (0x1)

/* [CLASS] ================================================================== */

class RobotCommunication {
 public: /* [PUBLIC] -------------------------------------------------------- */
  void init(uint32_t baudrate = 115200);

  /**
   * @brief
   *
   */
  typedef void (*robocom_callback_t)(uint8_t* data, uint8_t size);

  /**
   * @brief Attaches function to given uint8_t-based command.
   *
   * The given callback/function pointer must be of signature `void (uint8_t*,
   * uint8_t)`.
   *
   * `uint8_t*` data pointer
   *
   * `uint8_t` size of data
   *
   *  !  Memory leakage is possible, when not handled correctly
   *  !  Function pointers can be overwritten
   *
   * @param command integer based command (limit is FUNCTION_LIST_SIZE (default:
   * 0..31))
   * @param callback function pointer which gets called, when command is called.
   * @return true Success when attached
   * @return false When command bigger or equal than FUNCTION_LIST_SIZE
   */
  bool attach(uint8_t command, robocom_callback_t callback);

  /**
   * @brief Placeholder function that represents the whole process
   * Grabs input and directly processes it. Defining `ENABLE_LOG` in
   * `definitions.h` enables verbose output to the USB.
   */
  void run(void);

  /**  TODO
   * @brief
   *
   * @param command
   * @param size
   * @param data
   */
  void send_request(uint8_t command, uint8_t size = 0, uint8_t* data = NULL);

  /**  TODO
   * @brief Sends error code using the given error code and send it
   *
   * @param error Code that get's reported.
   */
  void send_error(uint8_t error);

  /** TODO
   * @brief
   *
   * @param command
   * @param size
   * @param data
   */
  void send_response(uint8_t command, uint8_t size = 0, uint8_t* data = NULL);

 private: /* [PRIVATE] ------------------------------------------------------ */
  /* [ENUM, STRUCT] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /**
   * @brief States for the state machines used in the evaluation of the
   * package.
   */
  typedef enum robocom_state_e {
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
  } robocom_state_t;

  /**
   * @brief Starting Symbol Type. Each package type is defined by its
   * starting symbol (and the enum uses the respective ASCII values).
   *
   * This is done for easier handling in the evaluation process.
   */
  typedef enum robocom_package_type_e {
    NONE,
    REQUEST = '?',
    RESPONSE = '!',
    ERROR = 'x'
  } robocom_package_type_t;

  /**
   * @brief
   *
   */
  typedef struct robocom_index_s {
    int16_t command;
    int16_t checksum;
    int16_t data;
  } robocom_index_t;

  /* [VARIABLES] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  uint8_t data_length = 0;
  uint8_t package_size = 0;
  alarm_id_t alarm_id = -1;

  robocom_index_t index = {
      /*command*/ -1,
      /*checksum*/ -1,
      /*data*/ -1};

  robocom_state_t state = AWAIT_START;
  robocom_package_type_t type = NONE;

  /**
   * @brief
   *
   */
  uint8_t buffer_index = 0;

  /**
   * @brief
   *
   */
  uint8_t buffer[BUFFER_SIZE];

  /**
   * 'robocom_function_list' is a function pointer array which is used
   * to attach functions with an command (via integer value).
   *
   * `uint8_t* data` contains the pointer to the given data, while
   * `uint8_t size` represents the size of the data package.
   *
   *  !  Memory Leakage is possible, when not handled correctly
   */
  robocom_callback_t function_list[FUNCTION_LIST_SIZE];

  /* [FUNCTIONS] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /**
   * @brief
   *
   *  NOTE  To change the interface used, rewrite this function
   *
   * @param byte Data that is sent.
   */
  void send_data(uint8_t byte);

  /**
   * @brief
   *
   * @param value
   * @param size
   */
  static void default_function(uint8_t* value, uint8_t size);

  /**
   * @brief
   *
   */
  void clear_function_list(void);

  /**
   * @brief
   *
   */
  void clear_buffer(void);

  /**
   * @brief
   *
   */
  void reset_state_machine(void);

  /* [watchdog] */
  static alarm_id_t watchdog_start(RobotCommunication* sender);
  static void watchdog_cancel(alarm_id_t alarm_id);
  static int64_t watchdog_callback(alarm_id_t id, void* user_data);
};

#endif  // ROBOCOM_H
