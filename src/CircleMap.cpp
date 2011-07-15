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

#include "CircleMap.hpp"
#include "CircleTopology.hpp"

using namespace sonotopy;
using namespace std;

CircleMap::CircleMap(const AudioParameters &_audioParameters,
		     const CircleMapParameters &_circleMapParameters)
  : SpectrumMap(new CircleTopology(_circleMapParameters.numNodes),
		_audioParameters,
		_circleMapParameters)
{
}

float CircleMap::getAngle() {
  moveTopologyCursorTowardsWinner();
  return ((CircleTopology*)topology)->getCursorAngle();
}

void CircleMap::write(ofstream &f) const {
  f << topology->getNumNodes() << endl;
  som->writeModelData(f);
}
