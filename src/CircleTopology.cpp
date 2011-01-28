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

#include "CircleTopology.hpp"
#include <math.h>

using namespace sonotopy;

CircleTopology::CircleTopology(unsigned int _numNodes) : Topology()
{
  numNodes = _numNodes;
  maxDistance = M_PI;
  fullAngle = 2 * M_PI;

  for(unsigned int i = 0; i < numNodes; i++) {
    Node node;
    node.angle = (float) i / numNodes * fullAngle;
    nodes.push_back(node);
  }
}

unsigned int CircleTopology::getNumNodes() {
  return numNodes;
}

float CircleTopology::getDistance(unsigned int sourceNodeId, unsigned int targetNodeId) {
  Node sourceNode = getNode(sourceNodeId);
  Node targetNode = getNode(targetNodeId);
  float angularDistance = fabs(sourceNode.angle - targetNode.angle);
  if(angularDistance > maxDistance)
    angularDistance = fullAngle - angularDistance;
  return angularDistance / maxDistance;
}

CircleTopology::Node CircleTopology::getNode(unsigned int nodeId) {
  return nodes[nodeId];
}

float CircleTopology::getCursorAngle() {
  return cursorAngle;
}

void CircleTopology::placeCursorAtNode(unsigned int nodeId) {
  cursorAngle = nodes[nodeId].angle;
}

void CircleTopology::moveCursorTowardsNode(unsigned int nodeId, float amount) {
  float targetAngle = nodes[nodeId].angle;
  if(fabsf(cursorAngle - targetAngle) < M_PI)
    cursorAngle += (targetAngle - cursorAngle) * amount;
  else
    cursorAngle -= (2 * M_PI - targetAngle + cursorAngle) * amount;
}
