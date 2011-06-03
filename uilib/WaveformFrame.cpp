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

#include "WaveformFrame.hpp"

WaveformFrame::WaveformFrame(const float *_buffer, unsigned long _bufferSize) {
  buffer = _buffer;
  bufferSize = _bufferSize;
}

void WaveformFrame::render() {
  glColor3f(1.0f, 1.0f, 1.0f);
  static float x;
  glShadeModel(GL_FLAT);
  glBegin(GL_POINTS);
  for(int i = 0; i < width; i++) {
    x = buffer[(int) (bufferSize * i / width)];
    vertex2i(i, (int) ((x + 1) / 2 * height));
  }
  glEnd();
}

