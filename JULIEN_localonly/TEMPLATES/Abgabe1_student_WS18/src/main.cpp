/*
 *  Copyright (C) 2015   Malte Brunn
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "typedef.hpp"
#include "compute.hpp"
#include "geometry.hpp"
#include "parameter.hpp"
#include "grid.hpp"
#include "iterator.hpp"
#include <cstdlib>

#ifdef USE_DEBUG_VISU
#include "visu.hpp"
#endif // USE_DEBUG_VISU
#ifdef USE_VTK
#include "vtk.hpp"
#include <cerrno>
#include <sys/stat.h>
#endif // USE_VTK

int main (int argc, char** argv) {
  // Create parameter and geometry instances with default values
  std::cout << "Reading input values... \n" << std::endl;
  Parameter param;
  Geometry geom;

  // testing successful: parameters and geometry
  // real_t rey = param.Re();
  // real_t omg = param.Omega();
  // real_t a = param.Alpha();
  // std::cout << rey << " " << omg << " " << a << "\n";
  // index_t size1 = geom.Size()[0];
  // index_t size2 = geom.Size()[1];
  // std::cout << rey << " " << size1 << " " << size2 << " end \n";


  // Create the fluid solver
  Compute comp(&geom, &param);
  std::cout << "Solving problem... \n" << std::endl;

#ifdef USE_VTK
  if (mkdir("out", 0777) != 0 && errno != EEXIST) {
    std::cerr << "Could not create output directory 'out'.\n";
    return EXIT_FAILURE;
  }
  struct stat output_info;
  if (stat("out", &output_info) != 0 || !S_ISDIR(output_info.st_mode)) {
    std::cerr << "Output path 'out' is not a directory.\n";
    return EXIT_FAILURE;
  }
#endif // USE_VTK

  // Create and initialize the visualization
#ifdef USE_DEBUG_VISU
  // Create and initialize the visualization
  Renderer visu(geom.Length(), geom.Mesh());
  visu.Init(800, 800);
#endif // USE_DEBUG_VISU

#ifdef USE_VTK
  // Create a VTK generator
  VTK vtk(geom.Mesh(),geom.Length());
#endif // USE_VTK

#ifdef USE_DEBUG_VISU
  const Grid* visugrid;
  visugrid = comp.GetVelocity();
#endif // USE_DEBUG_VISU

  // Run the time steps until the end is reached
  bool run = true;
  uint32_t timestep = 0;
  while (comp.GetTime() < param.Tend() && run) {
#ifdef USE_DEBUG_VISU
    // Render and check if window is closed
    switch (visu.Render(visugrid)) {
    case -1:
      run = false;
      break;
    case 0:
      visugrid = comp.GetVelocity();
      break;
    case 1:
      visugrid = comp.GetU();
      break;
    case 2:
      visugrid = comp.GetV();
      break;
    case 3:
      visugrid = comp.GetP();
      break;
    default:
      break;
    };
#endif // DEBUG_VISU

    comp.TimeStep((timestep + 1) % 10 == 0);

#ifdef USE_VTK
    vtk.Init("out/output", comp.GetTime());
    vtk.AddScalar("pressure", comp.GetP());
    vtk.AddField("velocity", comp.GetU(), comp.GetV());
    vtk.Finish();
#endif // USE_VTK

    ++timestep;
  }
  return 0;
}
