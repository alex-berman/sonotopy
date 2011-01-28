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

#ifndef _Topology_hpp_
#define _Topology_hpp_

#include <vector>

namespace sonotopy {

class Topology {
public:
  typedef struct {
    unsigned int nodeId;
    double strength; // from 0 to 1 where 1 is nearest neighbour
  } Neighbour;

  void getNeighbours(unsigned int nodeId, std::vector<Neighbour> &);
  void setVicinityFactor(float);
  virtual unsigned int getNumNodes() { return 0; }
  virtual float getDistance(unsigned int sourceNodeId, unsigned int targetNodeId) { return 0.0f; }
  virtual void placeCursorAtNode(unsigned int nodeId) {}
  virtual void moveCursorTowardsNode(unsigned int nodeId, float amount) {}

private:
  float vicinityFactor;
};

}

#endif
