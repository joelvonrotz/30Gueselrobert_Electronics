#ifndef SCARA_H
#define SCARA_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/double.h"
#include "scara_constants.h"

class SCARA
{
private:
  double l1;
  double l2;
  double l3;

  /* [CONSTANTS] */
  const double lookup_chunks[3] = {1, 2, 3};

public:
  SCARA(double length1, double length2, double length3);

  void calculateAngles(int32_t x, int32_t y, double *angles);
};

#endif /* SCARA_H */
