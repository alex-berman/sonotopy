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

#include "SmoothGridMapFrame.hpp"

SmoothGridMapFrame::SmoothGridMapFrame(GridMap *_gridMap, ColorScheme *_colorScheme) {
  gridMap = _gridMap;
  colorScheme = _colorScheme;
  gridMapWidth = gridMap->getParameters().gridWidth;
  gridMapHeight = gridMap->getParameters().gridHeight;
}

void SmoothGridMapFrame::render() {
  static int x1, x2, py1, py2;
  glShadeModel(GL_SMOOTH);
  for(int y = 0; y < gridMapHeight-1; y++) {
    for(int x = 0; x < gridMapWidth-1; x++) {
      x1 = (int) (width * x / (gridMapWidth-1));
      x2 = (int) (width * (x+1) / (gridMapWidth-1));
      py1 = (int) (y * height / (gridMapHeight-1));
      py2 = (int) ((y+1) * height / (gridMapHeight-1));
      glBegin(GL_POLYGON);
      setColorFromActivationPattern(x, y);
      vertex2i(x1, py1);
      setColorFromActivationPattern(x, y+1);
      vertex2i(x1, py2);
      setColorFromActivationPattern(x+1, y+1);
      vertex2i(x2, py2);
      setColorFromActivationPattern(x+1, y);
      vertex2i(x2, py1);
      setColorFromActivationPattern(x, y);
      vertex2i(x1, py1);
      glEnd();
    }
  }
}

void SmoothGridMapFrame::setColorFromActivationPattern(int x, int y) {
  static Color color;
  float v = gridMap->getActivation((unsigned int)x, (unsigned int)y);
  color = colorScheme->getColor(v);
  glColor3f(color.r, color.g, color.b);
}
