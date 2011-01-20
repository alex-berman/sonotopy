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

#include "Topology.hpp"

using namespace sonotopy;

void Topology::setVicinityFactor(float _vicinityFactor) {
  vicinityFactor = _vicinityFactor;
}

void Topology::getNeighbours(unsigned int nodeId, std::vector<Neighbour> &neighbours) {
  neighbours.clear();
  if(vicinityFactor > 0) {
    unsigned int numNodes = getNumNodes();
    float distance;
    Neighbour neighbour;
    for(unsigned int neighbourId = 0; neighbourId < numNodes; neighbourId++) {
      if(neighbourId != nodeId) {
        distance = getDistance(nodeId, neighbourId);
        if(distance < vicinityFactor) {
          neighbour.nodeId = neighbourId;
          neighbour.strength = (float) (vicinityFactor - distance) / vicinityFactor;
          neighbours.push_back(neighbour);
        }
      }
    }
  }
}
