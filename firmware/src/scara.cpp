#include <stdio.h>
#include "scara.h"

SCARA::SCARA(double length1, double length2, double length3)
    : l1(length1), l2(length2), l3(length3)
{
}

/**
 * @brief Calculates the appropriate angles for the SCARA arm in degrees
 *
 * @param x
 * @param y
 * @param angles 3-Element Array containing angles [α1,α2,α3] that get updated
 */
void SCARA::calculateAngles(int32_t goal_x, int32_t goal_y, double *angles)
{
  double x = (double)goal_x;
  double y = (double)goal_y;

  double theta1, theta2, theta3;

  double lp = sqrt(pow(x, 2) + pow(y, 2));

  /* Get Chunk-Angle for easier calculation of α2 & α3 */
  // a1 = this->lookup_chunks[index];
  theta1 = PI * 1 / 2; // angles are in radians

  /* [] */
  double x1 = this->l1 * cos(theta1);
  double y1 = this->l1 * sin(theta1);

  /* [] */
  double lp1 = sqrt(pow(x - x1, 2) + pow(y - y1, 2));

  // Calculate the slope so that the corresponding angle calculation can be used.
  double m = (x1 != 0) ? y1 / x1 : 1e+20;

  theta3 = acos((pow(this->l2, 2) + pow(this->l3, 2) - pow(lp1, 2)) / (2 * this->l2 * this->l3));

  // [Conditions for check] ===================================
  if (((m >= 0) && (m * x <= y)) ||
      ((m <= 0) && (m * x >= y)))
  {
    /* P IS LEFT OF ANGLE  θ1 */
    // printf("P is left of Line at angle θ1!\n");

    /* [] */
    double alpha = acos((pow(lp1, 2) + pow(this->l2, 2) - pow(this->l3, 2)) / (2 * lp1 * this->l2));

    double theta2a = acos((pow(lp1, 2) + pow(this->l1, 2) - pow(lp, 2)) / (2 * lp1 * this->l1));

    theta2 = theta2a - alpha;
  }
  else
  {
    /* P IS RIGHT OF ANGLE  θ1 */
    // printf("P is right of Line at angle θ1!\n");

    /* [] */

    double alpha = acos((pow(lp1, 2) + pow(this->l1, 2) - pow(lp, 2)) / (2 * this->l1 * lp1));

    double theta2a = acos((pow(lp1, 2) + pow(this->l2, 2) - pow(this->l3, 2)) / (2 * lp1 * this->l2));

    theta2 = theta2a + alpha;
  }

  angles[INDEX_θ1] = theta1;
  angles[INDEX_θ2] = theta2;
  angles[INDEX_θ3] = theta3;
}