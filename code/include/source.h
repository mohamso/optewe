/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 15, 2016
 */

#ifndef SOURCE_H
#define SOURCE_H

#include "fd3d.h"
#include "model3d.h"

void insert_stress_source(std::shared_ptr<fdm3d_t> waves, 
		          real* source, 
			  const int _x, 
			  const int _y, 
			  const int _z, 
			  const int it);

void insert_force_source(std::shared_ptr<fdm3d_t> waves,
                         real* source,
                         std::shared_ptr<model3d_t> model, 
			 const int _x, 
			 const int _y, 
			 const int _z, 
                         const int it,
                         const int direction,
                         const int type);

#endif // SOURCE_H
