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

#include "DisjointGridMap.hpp"

using namespace sonotopy;
using namespace std;

DisjointGridMap::DisjointGridMap(const AudioParameters &_audioParameters,
				 const GridMapParameters &_gridMapParameters,
				 const vector<DisjointGridTopology::Node> &nodes)
  : GridMap(_audioParameters,
	    _gridMapParameters,
	    new DisjointGridTopology(_gridMapParameters.gridWidth,
				     _gridMapParameters.gridHeight,
				     nodes))
{
  for(int y = 0; y < _gridMapParameters.gridHeight; y++)
    for(int x = 0; x < _gridMapParameters.gridWidth; x++)
      rectActivationPattern.push_back(0);
}

const SOM::ActivationPattern* DisjointGridMap::getActivationPattern() {
  SOM::ActivationPattern::iterator i = rectActivationPattern.begin();
  for(int y = 0; y < gridMapParameters.gridHeight; y++)
    for(int x = 0; x < gridMapParameters.gridWidth; x++)
      *i++ = 0;
      
  int n = 0;
  const SOM::ActivationPattern* nodalPattern = SpectrumMap::getActivationPattern();
  SOM::ActivationPattern::const_iterator j = rectActivationPattern.begin();
  unsigned int x, y;
  for(j = nodalPattern->begin(); j != nodalPattern->end(); j++) {
    ((DisjointGridTopology*)topology)->idToGridCoordinates(n, x, y);
    rectActivationPattern[y * gridMapParameters.gridWidth + x] = *j;
    n++;
  }

  return &rectActivationPattern;
}
