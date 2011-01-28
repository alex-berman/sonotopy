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

#ifndef _RectGridTopology_hpp_
#define _RectGridTopology_hpp_

#include "Topology.hpp"

namespace sonotopy {

class RectGridTopology : public Topology {
public:
  typedef struct {
    unsigned int x;
    unsigned int y;
  } Node;

  RectGridTopology(unsigned int gridWidth, unsigned int gridHeight);
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

private:
  unsigned int gridWidth;
  unsigned int gridHeight;
  unsigned int numNodes;
  unsigned int maxDistance;
  float cursorX, cursorY;
};

}

#endif
