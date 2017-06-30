/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 15, 2016
 */

#ifndef VTK_H
#define VTK_H

#include <fstream>
#include <cstring>
#include <typeinfo>

#include "fd3d.h"

void export_to_vtk(std::shared_ptr<fdm3d_t> waves, const std::string& filename);

#endif // VTK_H