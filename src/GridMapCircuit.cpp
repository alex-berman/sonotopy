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

#include "GridMapCircuit.hpp"
#include "RectGridTopology.hpp"

using namespace sonotopy;

GridMapCircuit::GridMapCircuit(const AudioParameters &_audioParameters,
                               const GridMapCircuitParameters &_gridMapCircuitParameters)
                               : SpectrumMapCircuit(new RectGridTopology(_gridMapCircuitParameters.gridWidth,
									 _gridMapCircuitParameters.gridHeight),
                                                    _audioParameters,
                                                    _gridMapCircuitParameters)
{
}

float GridMapCircuit::getActivation(unsigned int x, unsigned int y) {
  unsigned int nodeId = ((RectGridTopology*) topology)->gridCoordinatesToId(x, y);
  getActivationPattern();
  return (*currentActivationPattern)[nodeId];
}
