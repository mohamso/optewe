/* Original author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Modified by: Espen B. Raknes <espen.raknes@ntnu.no>
 * Date: September 15, 2016
 */

#include "vtk.h"

std::string type_name() {
  std::string type = typeid(real).name();

  if (type.compare("f") == 0) {
    return "FLOAT";
  } else {
    return "DOUBLE";
  }
}

void export_to_vtk(std::shared_ptr<fdm3d_t> waves, const std::string& filename) {

  int Nx = waves->nx_ghost;
  int Ny = waves->ny_ghost;
  int Nz = waves->nz_ghost;

  std::ofstream vtk_file(filename + ".vtk", std::ofstream::out);

  vtk_file << "# vtk DataFile Version 3.0\n";
  vtk_file << "vtk output\n";
  vtk_file << "ASCII\n";
  vtk_file << "DATASET STRUCTURED_GRID\n";
  vtk_file << "DIMENSIONS ";
  vtk_file << Nx << "" << " " << Ny << " " << Nz << "\n";
  vtk_file << "POINTS " << Nx * Ny * Nz << " " << type_name() << "\n";

  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << i * waves->dx << " " << j * waves->dy << " " << k * waves->dz << "\n";
      }
    }
  }

  vtk_file << "POINT_DATA " << Nx * Ny * Nz << "\n";
  vtk_file << "SCALARS Sxx " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->sxx[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "SCALARS Syy " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->syy[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "SCALARS Szz " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->szz[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "SCALARS Sxz " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->sxz[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "SCALARS Sxy " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->sxy[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "SCALARS Syz " << type_name() << " 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->syz[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }
  
  vtk_file << "VECTORS Velocity " << type_name() << "\n";
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        vtk_file << waves->vx[idx(Nx, Ny, i, j, k)] << " ";
        vtk_file << waves->vy[idx(Nx, Ny, i, j, k)] << " ";
        vtk_file << waves->vz[idx(Nx, Ny, i, j, k)] << "\n";
      }
    }
  }

  vtk_file.close();
}
