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

#ifndef _GlWindow_hpp_
#define _GlWindow_hpp_

#include <GL/glut.h>

class GlWindow {
public:
  GlWindow(int argc, char **argv, int width, int height);
  void glReshape(int width, int height);
  void glDisplay();
  void glText(int x, int y, const char *text);
  virtual void resizedWindow() {}
  virtual void display() {}
  virtual void glKeyboard(unsigned char key, int x, int y) {}
  virtual void glSpecial(int key, int x, int y) {}

protected:
  int windowWidth, windowHeight;
};

#endif
