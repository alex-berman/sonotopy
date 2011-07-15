// Copyright (C) 2011 Alexander Berman
//
// Sonotopy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef GRIDMAP_HPP
#define GRIDMAP_HPP

#include "SpectrumMap.hpp"
#include "AudioParameters.hpp"
#include "GridMapParameters.hpp"
#include "SOM.hpp"
#include <fstream>

namespace sonotopy {

class GridMap : public SpectrumMap
{
public:
  GridMap(const AudioParameters &, const GridMapParameters &);
  float getActivation(unsigned int x, unsigned int y);
  const float* getModel(unsigned int x, unsigned int y) const;
  void getCursor(float &x, float &y);
  const GridMapParameters getParameters() const;
  void write(std::ofstream &f) const;

private:
  GridMapParameters gridMapParameters;
};

}

#endif // GRIDMAP_HPP
