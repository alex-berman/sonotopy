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

#include "GridMapFrame.hpp"
#include <math.h>

GridMapFrame::GridMapFrame(GridMap *_gridMap) {
  gridMap = _gridMap;
  gridMapWidth = gridMap->getParameters().gridWidth;
  gridMapHeight = gridMap->getParameters().gridHeight;
}

void GridMapFrame::render() {
  glShadeModel(GL_FLAT);
  renderActivationPattern();
  renderCursor();
}

void GridMapFrame::renderActivationPattern() {
  static float v;
  static int x1, x2, py1, py2;
  activationPattern = gridMap->getActivationPattern();
  SOM::ActivationPattern::const_iterator activationPatternIterator =
    activationPattern->begin();
  for(int y = 0; y < gridMapHeight; y++) {
    for(int x = 0; x < gridMapWidth; x++) {
      v = *activationPatternIterator;
      v = pow(v, activationPatternContrast);
      glColor3f(v, v, v);
      glBegin(GL_POLYGON);
      x1 = (int) (width * x / gridMapWidth);
      x2 = (int) (width * (x+1) / gridMapWidth);
      py1 = (int) (y * height / gridMapHeight);
      py2 = (int) ((y+1) * height / gridMapHeight);
      vertex2i(x1, py1);
      vertex2i(x1, py2);
      vertex2i(x2, py2);
      vertex2i(x2, py1);
      vertex2i(x1, py1);
      glEnd();
      activationPatternIterator++;
    }
  }
}

void GridMapFrame::renderCursor() {
  float wx, wy;
  int x1, y1, x2, y2;
  int s = (int) (width / gridMapWidth / 2);
  gridMap->getCursor(wx, wy);
  x1 = (int) (width  * wx) - s;
  y1 = (int) (height * wy) - s;
  x2 = (int) (width  * wx) + s;
  y2 = (int) (height * wy) + s;
  glColor3f(1, 0, 0);
  glBegin(GL_POLYGON);
  vertex2i(x1, y1);
  vertex2i(x2, y1);
  vertex2i(x2, y2);
  vertex2i(x1, y2);
  vertex2i(x1, y1);
  glEnd();
}

