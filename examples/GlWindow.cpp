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

#include "GlWindow.hpp"
#include <stdio.h>

GlWindow *_glWindow;

void GlWindow_display() {
  _glWindow->glDisplay();
}

void GlWindow_reshape(int width, int height) {
  _glWindow->glReshape(width, height);
}

void GlWindow_keyboard(unsigned char key, int x, int y) {
  _glWindow->glKeyboard(key, x, y);
}

void GlWindow_special(int key, int x, int y) {
  _glWindow->glSpecial(key, x, y);
}

GlWindow::GlWindow(int argc, char **argv, int _width, int _height) {
  _glWindow = this;
  width = _width;
  height = _height;

  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize (width, height);
  //glutInitWindowPosition (0, 0);
  glutCreateWindow (argv[0]);
  glutDisplayFunc(GlWindow_display);
  glutIdleFunc(GlWindow_display);
  glutReshapeFunc(GlWindow_reshape);
  glutKeyboardFunc(GlWindow_keyboard);
  glutSpecialFunc(GlWindow_special);
}
