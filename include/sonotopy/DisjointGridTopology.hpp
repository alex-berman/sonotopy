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

#ifndef _DisjointGridTopology_hpp_
#define _DisjointGridTopology_hpp_

#include "Topology.hpp"
#include <vector>

namespace sonotopy {

class DisjointGridTopology : public Topology {
public:
  class Node {
  public:
    Node(unsigned int _x, unsigned int _y) {
      x = _x;
      y = _y;
    }
    unsigned int x;
    unsigned int y;
  };

  DisjointGridTopology(unsigned int gridWidth, unsigned int gridHeight,
		       const std::vector<Node> &nodes);
  unsigned int getNumNodes();
  float getDistance(unsigned int sourceNodeId, unsigned int targetNodeId);
  unsigned int getGridWidth() { return gridWidth; }
  unsigned int getGridHeight() { return gridHeight; }
  Node getNode(unsigned int nodeId);
  void placeCursorAtNode(unsigned int nodeId);
  void moveCursorTowardsNode(unsigned int nodeId, float amount);
  unsigned int gridCoordinatesToId(unsigned int x, unsigned int y);
  void idToGridCoordinates(unsigned int id, unsigned int &x, unsigned int &y);
  void getCursorPosition(float &x, float &y);
  bool containsCoordinates(unsigned int x, unsigned int y);

private:
  unsigned int gridWidth;
  unsigned int gridHeight;
  unsigned int numNodes;
  std::vector<Node> nodes;
  unsigned int maxDistance;
  float cursorX, cursorY;
};

}

#endif
