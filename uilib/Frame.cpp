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

#include "Frame.hpp"
#include <GL/glut.h>

Frame::Frame() {
  width = 0;
  height = 0;
  posLeft = 0;
  posTop = 0;
  setPadding(2);
  setBorderWidth(1);
}

void Frame::setPadding(int _padding) {
  padding = _padding;
  margin = borderWidth + padding;
}

void Frame::setBorderWidth(int _borderWidth) {
  borderWidth = _borderWidth;
  margin = borderWidth + padding;
}

void Frame::setSize(int _outerWidth, int _outerHeight) {
  outerWidth = _outerWidth;
  outerHeight = _outerHeight;
  width = outerWidth - margin*2;
  height = outerHeight - margin*2;
}

void Frame::setPosition(int _posLeft, int _posTop) {
  posLeft = _posLeft;
  posTop = _posTop;
}

int Frame::getWidth() { return width; }
int Frame::getHeight() { return height; }
int Frame::getLeft() { return posLeft; }
int Frame::getBottom() { return posTop + height; }

void Frame::display() {
  drawBorder();
  render();
}

void Frame::drawBorder() {
  glLineWidth(1.0f);
  glColor3f(0.3f, 0.3f, 0.3f);
  drawRectangle(
    posLeft, posTop,
    outerWidth, outerHeight);
}

void Frame::drawRectangle(int left, int top, int width, int height) {
  int right = left + width;
  int bottom = top + height;
  glBegin(GL_LINE_STRIP);
  glVertex2i(left, top);
  glVertex2i(left, bottom);
  glVertex2i(right, bottom);
  glVertex2i(right, top);
  glVertex2i(left, top);
  glEnd();
}

void Frame::vertex2i(int x, int y) {
  glVertex2i(posLeft + margin + x, posTop + margin + y);
}

void Frame::vertex2f(float x, float y) {
  glVertex2f(posLeft + margin + x, posTop + margin + y);
}
