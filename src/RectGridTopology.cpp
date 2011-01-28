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

#include "RectGridTopology.hpp"
#include <math.h>
#include <cassert>

using namespace sonotopy;

RectGridTopology::RectGridTopology(unsigned int _gridWidth, unsigned int _gridHeight) : Topology()
{
  assert(_gridWidth != 0);
  assert(_gridWidth != 0);

  gridWidth = _gridWidth;
  gridHeight = _gridHeight;
  numNodes = gridWidth * gridHeight;

  maxDistance = gridWidth*gridWidth + gridHeight*gridHeight;
}

unsigned int RectGridTopology::getNumNodes() {
  return numNodes;
}

float RectGridTopology::getDistance(unsigned int sourceNodeId, unsigned int targetNodeId) {
  Node sourceNode = getNode(sourceNodeId);
  Node targetNode = getNode(targetNodeId);
  int dx = sourceNode.x - targetNode.x;
  int dy = sourceNode.y - targetNode.y;
  return (float) (dx*dx + dy*dy) / maxDistance;
}

void RectGridTopology::idToGridCoordinates(unsigned int id, unsigned int &x, unsigned int &y) {
  y = id / gridWidth;
  x = id - y * gridWidth;
}

unsigned int RectGridTopology::gridCoordinatesToId(unsigned int x, unsigned int y) {
  return y * gridWidth + x;
}

RectGridTopology::Node RectGridTopology::getNode(unsigned int nodeId) {
  Node node;
  idToGridCoordinates(nodeId, node.x, node.y);
  return node;
}

void RectGridTopology::placeCursorAtNode(unsigned int nodeId) {
  Node node = getNode(nodeId);
  cursorX = node.x;
  cursorY = node.y;
}

void RectGridTopology::moveCursorTowardsNode(unsigned int nodeId, float amount) {
  Node targetNode = getNode(nodeId);
  cursorX += (targetNode.x - cursorX) * amount;
  cursorY += (targetNode.y - cursorY) * amount;
}

void RectGridTopology::getCursorPosition(float &x, float &y) {
  x = cursorX;
  y = cursorY;
}
