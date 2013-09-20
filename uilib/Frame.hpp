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

#ifndef _Frame_hpp_
#define _Frame_hpp_

#include <GL/glut.h>

class Frame {
public:
  const static float activationPatternContrast;
  Frame();
  void setSize(int width, int height);
  void setPosition(int width, int height);
  void setPadding(int);
  void setBorderWidth(int);
  void display();
  virtual void render() {}
  int getWidth();
  int getHeight();
  int getLeft();
  int getBottom();
  void drawBorder();
  void drawRectangle(int left, int top, int width, int height);
  void vertex2f(float x, float y);
  void vertex2i(int x, int y);

protected:
  int posLeft, posTop;
  int width, height;
  int margin;
  int outerWidth, outerHeight;
  int padding;
  int borderWidth;
};

#endif
