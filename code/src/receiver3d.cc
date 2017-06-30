/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 19, 2016
 */

#include "receiver3d.h"

std::shared_ptr<receiver3d_t> receiver3d_setup(const int _n,
                                                const int _nt,
                                                const bool _P,
                                                const bool _Vx,
                                                const bool _Vy,
                                                const bool _Vz) {
  // Create receiver structure
  std::shared_ptr <receiver3d_t> rec((receiver3d_t*) malloc(sizeof(receiver3d_t)), free_ptr());

  // Setup struct
  rec->n = _n;
  rec->nt = _nt;
  rec->P = _P;
  rec->Vx = _Vx;
  rec->Vy = _Vy;
  rec->Vz = _Vz;

  // Allocate arrays
  size_t num_bytes = sizeof(real) * ((rec->n) * (rec->nt));
  size_t num_bytes_pos = sizeof(int) * (rec->n);

  if (rec->P) {
	  rec->p = (real*) malloc(num_bytes);
	  std::memset(rec->p, 0, num_bytes);
  }
  if (rec->Vx) {
	  rec->vx = (real*) malloc(num_bytes);
	  std::memset(rec->vx, 0, num_bytes);
  }
  if (rec->Vy) {
	  rec->vy = (real*) malloc(num_bytes);
	  std::memset(rec->vy, 0, num_bytes);
  }
  if (rec->Vz) {
	  rec->vz = (real*) malloc(num_bytes);
	  std::memset(rec->vz, 0, num_bytes);
  }

  rec->x = (int*) malloc(num_bytes_pos);
  rec->y = (int*) malloc(num_bytes_pos);
  rec->z = (int*) malloc(num_bytes_pos);

  // Reset arrays
  std::memset(rec->x, 0, num_bytes_pos);
  std::memset(rec->y, 0, num_bytes_pos);
  std::memset(rec->z, 0, num_bytes_pos);

  return rec;
}

void save_receivers(std::shared_ptr <receiver3d_t> rec, std::shared_ptr <fdm3d_t> waves, const int _it) {

  for (int i = 0; i < rec->n; i++) {
    if (rec->P) {
      rec->p[i * (rec->nt) + _it] = kOneThird
          * (waves->sxx[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]
              + waves->syy[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]
              + waves->szz[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]);
    }

    if (rec->Vx) {
      rec->vx[i * (rec->nt) + _it] =
          (waves->vx[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]);
    }

    if (rec->Vy) {
      rec->vy[i * (rec->nt) + _it] =
          (waves->vy[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]);
    }

    if (rec->Vz) {
      rec->vz[i * (rec->nt) + _it] =
          (waves->vz[idx(waves->nx_ghost, waves->ny_ghost, rec->x[i], rec->y[i], rec->z[i])]);
    }
  }
}

void write_receiver_file(std::shared_ptr <receiver3d_t> rec,
                         std::shared_ptr <dims_t> dims) {

  std::ofstream rec_file("receivers.csv", std::ofstream::out);

  rec_file << rec->n << "\n";
  rec_file << rec->nt << "\n";

  for (int i = 0; i < (rec->n); i++) {
    rec_file << rec->x[i] * dims->dx << " " << rec->y[i] * dims->dy << " " << rec->z[i] * dims->dz << "\n";

    if (rec->P) {
      rec_file << "P\n";
      for (int it = 0; it < (rec->nt); it++) {
        rec_file << rec->p[it+(i*rec->nt)] << "\n";
      }
    }
    if (rec->Vx) {
      rec_file << "Vx\n";
      for (int it = 0; it < (rec->nt); it++) {
        rec_file << rec->vx[it+(i*rec->nt)] << "\n";
      }
    }
    if (rec->Vy) {
      rec_file << "Vy\n";
      for (int it = 0; it < (rec->nt); it++) {
        rec_file << rec->vy[it+(i*rec->nt)] << "\n";
      }
    }
    if (rec->Vz) {
      rec_file << "Vz\n";
      for (int it = 0; it < (rec->nt); it++) {
        rec_file << rec->vz[it+(i*rec->nt)] << "\n";
      }
    }
    rec_file << "\n";
  }

  rec_file.close();
}

int determine_receiver_value(const int Nz) {

  if (Nz == 64) {
    return 8;
  } else if (Nz == 128) {
    return 16;
  } else if (Nz == 256) {
    return 24;
  } else if (Nz == 512) {
    return 32;
  } else if (Nz == 1024) {
    return 40;
  } else {
    return 1;
  }

}

void setup_receiver_for_verification(std::shared_ptr <receiver3d_t> receiver,
                                     const int Nz,
                                     const int x_source,
                                     const int y_source,
                                     const int z_source) {
  if (Nz == 32) {
    setup32(receiver, x_source, y_source, z_source);
  }
  else if (Nz == 64) {
    setup64(receiver, x_source, y_source, z_source);
  } else if (Nz == 128) {
    setup128(receiver, x_source, y_source, z_source);
  } else if (Nz == 256) {
    setup256(receiver, x_source, y_source, z_source);
  } else if (Nz == 512) {
    setup512(receiver, x_source, y_source, z_source);
  } else if (Nz == 1024) {
    setup1024(receiver, x_source, y_source, z_source);
  }
}

void setup32(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  receiver->x[0] = x_source;
  receiver->y[0] = y_source;
  receiver->z[0] = z_source;
}

void setup64(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  receiver->x[0] = x_source + 20;
  receiver->y[0] = y_source + 20;
  receiver->z[0] = z_source + 20;
  
  receiver->x[1] = x_source - 20;
  receiver->y[1] = y_source + 20;
  receiver->z[1] = z_source + 20;
  
  receiver->x[2] = x_source - 20;
  receiver->y[2] = y_source - 20;
  receiver->z[2] = z_source + 20;
  
  receiver->x[3] = x_source + 20;
  receiver->y[3] = y_source - 20;
  receiver->z[3] = z_source + 20;
  
  receiver->x[4] = x_source + 20;
  receiver->y[4] = y_source + 20;
  receiver->z[4] = z_source - 20;
  
  receiver->x[5] = x_source - 20;
  receiver->y[5] = y_source + 20;
  receiver->z[5] = z_source - 20;
  
  receiver->x[6] = x_source - 20;
  receiver->y[6] = y_source - 20;
  receiver->z[6] = z_source - 20;
  
  receiver->x[7] = x_source + 20;
  receiver->y[7] = y_source - 20;
  receiver->z[7] = z_source - 20;
}

void setup128(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  setup64(receiver,x_source,y_source,z_source);
  
  receiver->x[8] = x_source + 40;
  receiver->y[8] = y_source + 40;
  receiver->z[8] = z_source + 40;
  
  receiver->x[9] = x_source - 40;
  receiver->y[9] = y_source + 40;
  receiver->z[9] = z_source + 40;
  
  receiver->x[10] = x_source - 40;
  receiver->y[10] = y_source - 40;
  receiver->z[10] = z_source + 40;
  
  receiver->x[11] = x_source + 40;
  receiver->y[11] = y_source - 40;
  receiver->z[11] = z_source + 40;
  
  receiver->x[12] = x_source + 40;
  receiver->y[12] = y_source + 40;
  receiver->z[12] = z_source - 40;
  
  receiver->x[13] = x_source - 40;
  receiver->y[13] = y_source + 40;
  receiver->z[13] = z_source - 40;
  
  receiver->x[14] = x_source - 40;
  receiver->y[14] = y_source - 40;
  receiver->z[14] = z_source - 40;
  
  receiver->x[15] = x_source + 40;
  receiver->y[15] = y_source - 40;
  receiver->z[15] = z_source - 40;
}


void setup256(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  setup128(receiver,x_source,y_source,z_source);

  receiver->x[16] = x_source + 100;
  receiver->y[16] = y_source + 100;
  receiver->z[16] = z_source + 100;
  
  receiver->x[17] = x_source - 100;
  receiver->y[17] = y_source + 100;
  receiver->z[17] = z_source + 100;
  
  receiver->x[18] = x_source - 100;
  receiver->y[18] = y_source - 100;
  receiver->z[18] = z_source + 100;
  
  receiver->x[19] = x_source + 100;
  receiver->y[19] = y_source - 100;
  receiver->z[19] = z_source + 100;
  
  receiver->x[20] = x_source + 100;
  receiver->y[20] = y_source + 100;
  receiver->z[20] = z_source - 100;
  
  receiver->x[21] = x_source - 100;
  receiver->y[21] = y_source + 100;
  receiver->z[21] = z_source - 100;
  
  receiver->x[22] = x_source - 100;
  receiver->y[22] = y_source - 100;
  receiver->z[22] = z_source - 100;
  
  receiver->x[23] = x_source + 100;
  receiver->y[23] = y_source - 100;
  receiver->z[23] = z_source - 100;
}

void setup512(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  setup256(receiver,x_source,y_source,z_source);

  receiver->x[24] = x_source + 200;
  receiver->y[24] = y_source + 200;
  receiver->z[24] = z_source + 200;
  
  receiver->x[25] = x_source - 200;
  receiver->y[25] = y_source + 200;
  receiver->z[25] = z_source + 200;
  
  receiver->x[26] = x_source - 200;
  receiver->y[26] = y_source - 200;
  receiver->z[26] = z_source + 200;
  
  receiver->x[27] = x_source + 200;
  receiver->y[27] = y_source - 200;
  receiver->z[27] = z_source + 200;
  
  receiver->x[28] = x_source + 200;
  receiver->y[28] = y_source + 200;
  receiver->z[28] = z_source - 200;
  
  receiver->x[29] = x_source - 200;
  receiver->y[29] = y_source + 200;
  receiver->z[29] = z_source - 200;
  
  receiver->x[30] = x_source - 200;
  receiver->y[30] = y_source - 200;
  receiver->z[30] = z_source - 200;
  
  receiver->x[31] = x_source + 200;
  receiver->y[31] = y_source - 200;
  receiver->z[31] = z_source - 200;
}

void setup1024(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source) {
  setup512(receiver,x_source,y_source,z_source);

  receiver->x[32] = x_source + 400;
  receiver->y[32] = y_source + 400;
  receiver->z[32] = z_source + 400;
  
  receiver->x[33] = x_source - 400;
  receiver->y[33] = y_source + 400;
  receiver->z[33] = z_source + 400;
  
  receiver->x[34] = x_source - 400;
  receiver->y[34] = y_source - 400;
  receiver->z[34] = z_source + 400;
  
  receiver->x[35] = x_source + 400;
  receiver->y[35] = y_source - 400;
  receiver->z[35] = z_source + 400;
  
  receiver->x[36] = x_source + 400;
  receiver->y[36] = y_source + 400;
  receiver->z[36] = z_source - 400;
  
  receiver->x[37] = x_source - 400;
  receiver->y[37] = y_source + 400;
  receiver->z[37] = z_source - 400;
  
  receiver->x[38] = x_source - 400;
  receiver->y[38] = y_source - 400;
  receiver->z[38] = z_source - 400;
  
  receiver->x[39] = x_source + 400;
  receiver->y[39] = y_source - 400;
  receiver->z[39] = z_source - 400;
}

void free_receiver_arrays(std::shared_ptr <receiver3d_t> rec) {
  if (rec->P) free(rec->p);
  if (rec->Vx) free(rec->vx);
  if (rec->Vy) free(rec->vy);
  if (rec->Vz) free(rec->vz);
  free(rec->x);
  free(rec->y);
  free(rec->z);
}
