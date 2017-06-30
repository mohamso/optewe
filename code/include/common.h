/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: January 16, 2012
 * Updated: September 3, 2016
 */

#ifndef COMMON_H
#define COMMON_H

typedef float real;

constexpr real kPI = 3.14159265358979323846;
constexpr real kOneThird = 0.3333333333;

inline int idx(int Nx, int Ny, int i, int j, int k) {
  int j_off = (Nx);
  int k_off = j_off * (Ny);

  int idx = i + j * j_off + k * k_off;

  return idx;
}

#endif // COMMON_H
