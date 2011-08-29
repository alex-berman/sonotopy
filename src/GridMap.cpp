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

#include "GridMap.hpp"
#include "RectGridTopology.hpp"

using namespace sonotopy;
using namespace std;

GridMap::GridMap(const AudioParameters &_audioParameters,
		 const GridMapParameters &_gridMapParameters)
  : SpectrumMap(new RectGridTopology(_gridMapParameters.gridWidth,
				     _gridMapParameters.gridHeight),
		_audioParameters,
		_gridMapParameters)
{
  gridMapParameters = _gridMapParameters;
}

GridMap::GridMap(const AudioParameters &_audioParameters,
		 const GridMapParameters &_gridMapParameters,
		 Topology *topology)
  : SpectrumMap(topology,
		_audioParameters,
		_gridMapParameters)
{
  gridMapParameters = _gridMapParameters;
}

float GridMap::getActivation(unsigned int x, unsigned int y) {
  unsigned int nodeId = ((RectGridTopology*) topology)->gridCoordinatesToId(x, y);
  getActivationPattern();
  return (*currentActivationPattern)[nodeId];
}

const float* GridMap::getModel(unsigned int x, unsigned int y) const {
  unsigned int nodeId = ((RectGridTopology*) topology)->gridCoordinatesToId(x, y);
  return som->getModel(nodeId);
}

void GridMap::getCursor(float &x, float &y) {
  static float gridX, gridY;
  moveTopologyCursorTowardsWinner();
  ((RectGridTopology*) topology)->getCursorPosition(gridX, gridY);
  x = (gridX + 0.5) / gridMapParameters.gridWidth;
  y = (gridY + 0.5) / gridMapParameters.gridHeight;
}

const GridMapParameters GridMap::getParameters() const {
  return gridMapParameters;
}

void GridMap::write(ofstream &f) const {
  f << gridMapParameters.gridWidth << endl;
  f << gridMapParameters.gridHeight << endl;
  som->writeModelData(f);
}
