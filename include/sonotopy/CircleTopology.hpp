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

#ifndef _CIRCLE_TOPOLOGY_HPP_
#define _CIRCLE_TOPOLOGY_HPP_

#include "Topology.hpp"
#include <vector>

namespace sonotopy {

class CircleTopology : public Topology {
public:
  typedef struct {
    float angle;
  } Node;

  CircleTopology(unsigned int numNodes);
  unsigned int getNumNodes();
  float getDistance(unsigned int sourceNodeId, unsigned int targetNodeId);
  Node getNode(unsigned int nodeId);

private:
  unsigned int numNodes;
  float maxDistance;
  float fullAngle;
  std::vector<Node> nodes;
};

}

#endif
