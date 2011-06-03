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

#include "CircleMapFrame.hpp"
#include <math.h>

CircleMapFrame::CircleMapFrame(CircleMap *_circleMap, BeatTracker *_beatTracker) {
  circleMap = _circleMap;
  beatTracker = _beatTracker;
  circleTopology = (CircleTopology*) circleMap->getTopology();
  numNodes = circleTopology->getNumNodes();
}

void CircleMapFrame::render() {
  static float c;
  static int x1, y1, x2, y2;
  int centreX = width / 2;
  int centreY = height / 2;
  int radius = (int) (width * 0.4);
  float angleSpan = 2 * M_PI / numNodes;
  glShadeModel(GL_FLAT);
  activationPattern = circleMap->getActivationPattern();
  SOM::ActivationPattern::const_iterator activationPatternIterator = activationPattern->begin();
  CircleTopology::Node node;
  for(int i = 0; i < numNodes; i++) {
    c = *activationPatternIterator;
    node = circleTopology->getNode(i);
    x1 = centreX + radius * cos(node.angle - angleSpan);
    y1 = centreY + radius * sin(node.angle - angleSpan);
    x2 = centreX + radius * cos(node.angle + angleSpan);
    y2 = centreY + radius * sin(node.angle + angleSpan);
    glBegin(GL_POLYGON);
    glColor3f(c, c, c);
    vertex2i(centreX, centreY);
    vertex2i(x1, y1);
    vertex2i(x2, y2);
    glEnd();
    activationPatternIterator++;
  }

  float angle = circleMap->getAngle();
  radius = width * (0.1 + beatTracker->getIntensity() * 0.3);
  x1 = centreX + radius * cos(angle);
  y1 = centreY + radius * sin(angle);
  glColor3f(1.0f, 0.0f, 0.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  vertex2i(centreX, centreY);
  vertex2i(x1, y1);
  glEnd();
}
