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

#include "SmoothCircleMapFrame.hpp"
#include <sonotopy/CircleTopology.hpp>
#include <math.h>

SmoothCircleMapFrame::SmoothCircleMapFrame(CircleMap *_circleMap) {
  circleMap = _circleMap;
  numNodes = ((CircleTopology *)circleMap->getTopology())->getNumNodes();
  angleIncrement = 2 * M_PI / numNodes;
}

void SmoothCircleMapFrame::render() {
  int numPoints = 100;
  float c;
  float a;
  int x, y;
  int centreX = width / 2;
  int centreY = height / 2;
  int radius = (int) (width * 0.4);

  activationPattern = circleMap->getActivationPattern();
  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(1,1,1);
  vertex2i(centreX, centreY);
  for(int i = 0; i <= numPoints; i++) {
    a = (float) i / numPoints * 2 * M_PI;
    c = getColorAtAngle(a);
    glColor3f(c, c, c);
    x  = centreX + radius * cos(a);
    y  = centreY + radius * sin(a);
    vertex2i(x, y);
  }
  glEnd();
}

float SmoothCircleMapFrame::getColorAtAngle(float angle) {
  angle = fmodf(angle, 2 * M_PI);
  int nodeId1 = (int) (angle / angleIncrement);
  int nodeId2 = (nodeId1 + 1) % numNodes;
  float nodeAngle1 = (float) nodeId1 / numNodes * 2 * M_PI;
  float nodeStrength1 = 1.0f - (angle - nodeAngle1) / angleIncrement;
  float nodeStrength2 = 1.0f - nodeStrength1;
  float nodeActivity1 = (*activationPattern)[nodeId1];
  float nodeActivity2 = (*activationPattern)[nodeId2];
  return nodeActivity1 * nodeStrength1 + nodeActivity2 * nodeStrength2;
}
