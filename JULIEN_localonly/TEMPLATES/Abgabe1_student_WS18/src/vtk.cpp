/*
 * Copyright (C) 2015   Malte Brunn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vtk.hpp"
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
//------------------------------------------------------------------------------
uint32_t VTK::_cnt = 0;
//------------------------------------------------------------------------------
VTK::VTK (const multi_real_t& h, const multi_real_t& length) :
	_h(h), _length(length) {
	_offset = multi_real_t(0.0);
	_handle = NULL;
  for (uint32_t d = 0; d < DIM; ++d) {
    _size[d] = (index_t)(_length[d]/_h[d]) + 1;
  }
}
//------------------------------------------------------------------------------
VTK::VTK (const multi_real_t& h, const multi_real_t& length, const multi_real_t& offset) :
	_h(h), _length(length), _offset(offset) {
	_handle = NULL;
  for (uint32_t d = 0; d < DIM; ++d) {
    _size[d] = (index_t)(_length[d]/_h[d]) + 1;
  }
}
//------------------------------------------------------------------------------
void VTK::Init (const char* path, const real_t& time) {
	if (_handle) {
		throw std::runtime_error("Cannot initialize a VTI file before finishing the previous file.");
	}

	std::ostringstream filename;
	filename << (path[0] ? path : "output") << "_"
	         << std::setw(4) << std::setfill('0') << _cnt << ".vti";
	_handle = fopen(filename.str().c_str(), "w");
	if (!_handle) {
		throw std::runtime_error("Could not open VTI output file: " + filename.str());
	}

	fprintf(_handle,"<?xml version=\"1.0\"?>\n");
	fprintf(_handle,"<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n");
	fprintf(_handle,"<ImageData WholeExtent=\"0 %u 0 %u 0 0\" Origin=\"%.17g %.17g 0\" Spacing=\"%.17g %.17g 1\">\n",
	        _size[0]-1, _size[1]-1, _offset[0], _offset[1], _h[0], _h[1]);
	fprintf(_handle,"<FieldData>\n");
	fprintf(_handle,"<DataArray type=\"Float64\" Name=\"TIME\" NumberOfTuples=\"1\" format=\"ascii\">%.17g</DataArray>\n",
	        time);
	fprintf(_handle,"</FieldData>\n");
	fprintf(_handle,"<Piece Extent=\"0 %u 0 %u 0 0\">\n", _size[0]-1, _size[1]-1);
	fprintf(_handle,"<PointData Scalars=\"pressure\" Vectors=\"velocity\">\n");
}
//------------------------------------------------------------------------------
void VTK::Finish () {
	if (!_handle) {
		throw std::runtime_error("Cannot finish a VTI file before initializing it.");
	}

	fprintf(_handle,"</PointData>\n");
	fprintf(_handle,"<CellData/>\n");
	fprintf(_handle,"</Piece>\n");
	fprintf(_handle,"</ImageData>\n");
	fprintf(_handle,"</VTKFile>\n");

	const bool write_error = ferror(_handle) != 0;
	const int close_error = fclose(_handle);
	_handle = NULL;
	if (write_error || close_error != 0) {
		throw std::runtime_error("Failed while writing a VTI output file.");
	}

	_cnt++;
}
//------------------------------------------------------------------------------
void VTK::AddScalar (const char* title, const Grid* grid) {
	if (!_handle) {
		throw std::runtime_error("Cannot add a scalar without an initialized VTI file.");
	}

	fprintf(_handle,"<DataArray Name=\"%s\" type=\"Float64\" format=\"ascii\">\n", title);

	multi_real_t pos;
	for (uint32_t y=0; y<_size[1]; ++y) {
		pos[1] = (real_t)y*_h[1]+_offset[1];
		for (uint32_t x=0; x<_size[0]; ++x) {
			pos[0] = (real_t)x*_h[0]+_offset[0];
			fprintf(_handle,"%.17g ", grid->Interpolate(pos));
		}
		fprintf(_handle,"\n");
	}

	fprintf(_handle,"</DataArray>\n");
}
//------------------------------------------------------------------------------
void VTK::AddField  (const char* title, const Grid* v1, const Grid* v2) {
	if (!_handle) {
		throw std::runtime_error("Cannot add a vector without an initialized VTI file.");
	}

	fprintf(_handle,"<DataArray Name=\"%s\" type=\"Float64\" format=\"ascii\" NumberOfComponents=\"3\">\n", title);

	multi_real_t pos;
	for (uint32_t y=0; y<_size[1]; ++y) {
		pos[1] = (real_t)y*_h[1]+_offset[1];
		for (uint32_t x=0; x<_size[0]; ++x) {
			pos[0] = (real_t)x*_h[0]+_offset[0];
			fprintf(_handle,"%.17g %.17g 0\n", v1->Interpolate(pos), v2->Interpolate(pos));
		}
	}

	fprintf(_handle,"</DataArray>\n");
}
//------------------------------------------------------------------------------
void VTK::AddField  (const char* title, const Grid* v1, const Grid* v2, const Grid* v3) {
	if (!_handle) {
		throw std::runtime_error("Cannot add a vector without an initialized VTI file.");
	}

	fprintf(_handle,"<DataArray Name=\"%s\" type=\"Float64\" format=\"ascii\" NumberOfComponents=\"3\">\n", title);

	multi_real_t pos;
	for (uint32_t y=0; y<_size[1]; ++y) {
		pos[1] = (real_t)y*_h[1]+_offset[1];
		for (uint32_t x=0; x<_size[0]; ++x) {
			pos[0] = (real_t)x*_h[0]+_offset[0];
			fprintf(_handle,"%.17g %.17g %.17g\n",
			        v1->Interpolate(pos), v2->Interpolate(pos), v3->Interpolate(pos));
		}
	}

	fprintf(_handle,"</DataArray>\n");
}
//------------------------------------------------------------------------------
