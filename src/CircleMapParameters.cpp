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

#include "CircleMapParameters.hpp"

using namespace sonotopy;

CircleMapParameters::CircleMapParameters()
{
  numNodes = 50;
  initialNeighbourhoodParameter = 1.0f;
  normalNeighbourhoodParameter = 0.2f;
  initialAdaptationTimeSecs = 0.03f;
  initialTrainingLengthSecs = 10.0;
  normalAdaptationTimeSecs = 5.0f;
}

