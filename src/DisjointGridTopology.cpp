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

#include "DisjointGridTopology.hpp"
#include <math.h>
#include <cassert>
#include <stdio.h> // TEMP

using namespace sonotopy;
using namespace std;

DisjointGridTopology::DisjointGridTopology(unsigned int _gridWidth, unsigned int _gridHeight,
					   const vector<Node> &_nodes) : Topology()
{
  assert(_gridWidth != 0);
  assert(_gridWidth != 0);

  gridWidth = _gridWidth;
  gridHeight = _gridHeight;
  nodes = _nodes;
  numNodes = nodes.size();

  maxDistance = gridWidth*gridWidth + gridHeight*gridHeight;
}

unsigned int DisjointGridTopology::getNumNodes() {
  return numNodes;
}

float DisjointGridTopology::getDistance(unsigned int sourceNodeId, unsigned int targetNodeId) {
  Node sourceNode = getNode(sourceNodeId);
  Node targetNode = getNode(targetNodeId);
  int dx = sourceNode.x - targetNode.x;
  int dy = sourceNode.y - targetNode.y;
  return (float) (dx*dx + dy*dy) / maxDistance;
}

void DisjointGridTopology::idToGridCoordinates(unsigned int id, unsigned int &x, unsigned int &y) {
  x = nodes[id].x;
  y = nodes[id].y;
}

unsigned int DisjointGridTopology::gridCoordinatesToId(unsigned int x, unsigned int y) {
  unsigned int n = 0;
  for(vector<Node>::iterator i = nodes.begin(); i != nodes.end(); i++) {
    if(i->x == x && i->y == y)
      return n;
    n++;
  }
  return numNodes;
}

DisjointGridTopology::Node DisjointGridTopology::getNode(unsigned int nodeId) {
  unsigned int x, y;
  idToGridCoordinates(nodeId, x, y);
  return Node(x, y);
}

void DisjointGridTopology::placeCursorAtNode(unsigned int nodeId) {
  Node node = getNode(nodeId);
  cursorX = node.x;
  cursorY = node.y;
}

void DisjointGridTopology::moveCursorTowardsNode(unsigned int nodeId, float amount) {
  Node targetNode = getNode(nodeId);
  cursorX += (targetNode.x - cursorX) * amount;
  cursorY += (targetNode.y - cursorY) * amount;
}

void DisjointGridTopology::getCursorPosition(float &x, float &y) {
  x = cursorX;
  y = cursorY;
}
