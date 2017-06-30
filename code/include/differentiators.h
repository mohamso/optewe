/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 13, 2016
*/

#ifndef KERNELS_DIFFERENTIATORS_H
#define KERNELS_DIFFERENTIATORS_H

#include "common.h"
#include <cstdlib>
#include <memory>
#include <cstring>
#include "mem_utils.h"

/* Todo: Add boundary treatment of the operators! Now they start half operator length in the model in all dimensions.
         Check that the indices in the array is correct related to the operator position!
	 Add padding to the array as option?
	 Write them as a single for loop and not nested for loops?
*/

// Operator half length
constexpr int half_length = 8;

// Weights in front of operators
constexpr real W[half_length] = {1.2627, -0.1312, 0.0412, -0.0170, 0.0076, -0.0034, 0.0014, -0.0005};

// Differentiation for dimension one (innermost dimension)
void dx_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);
void dx_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);

// Differentiation for dimension two (middle dimension)
void dy_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);
void dy_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);

// Differentiation for dimension three (outer dimension)
void dz_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);
void dz_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale);


/* Weights to have if other operators are used... DO NOT REMOVE!
L=1: 1.0029f

L=2: 1.1466f
     0.0498f

L=3: 1.2049f
    -0.0841f
     0.0100f

L=4: 1.2327f
    -0.1049f
     0.0211f
    -0.0038f
   
L=5  1.2463f
    -0.1163f
     0.0290f
    -0.0080f
     0.0018f
   
L=6  1.2542f
    -0.1233f
     0.0344f
    -0.0117f
     0.0039f
    -0.0011f
   
L=7  1.2593f
    -0.1280f
     0.0384f
    -0.0147f
     0.0059f
    -0.0022f
     0.0007f
   
L=8  1.2627f
    -0.1312f
     0.0412f
    -0.0170f
     0.0076f
    -0.0034f
     0.0014f
    -0.0005f
*/


#endif //KERNELS_DIFFERENTIATORS_H
