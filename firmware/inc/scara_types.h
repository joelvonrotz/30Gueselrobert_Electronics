#ifndef SCARA_TYPES_H
#define SCARA_TYPES_H

#include "pico/stdlib.h"

typedef struct scara_arm_s
{
  uint32_t x;
  uint32_t y;
  double length;
};

#endif /* SCARA_TYPES_H */
