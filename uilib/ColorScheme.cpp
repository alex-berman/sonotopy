// Copyright (C) 2013 Alexander Berman
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

#include "ColorScheme.hpp"
#include <math.h>

const float Grayscale::contrast = 5.0f;

Color Grayscale::getColor(float fraction) {
  float v = pow(fraction, contrast);
  return Color(v, v, v);
}


const float Stripes::contrast = 5.0f;

Color Stripes::getColor(float fraction) {
  float v = pow(sin(fraction * 10.0f), contrast);
  return Color(v, v, v);
}


Color Rainbow::getColor(float fraction) {
  return HSV_to_RGB(fraction, 1.0f, 1.0f);
}


Color ColorScheme::HSV_to_RGB(float h, float s, float v) {
  float r, g, b;

  int i = int(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  
  switch(i % 6) {
  case 0: r = v; g = t; b = p; break;
  case 1: r = q; g = v; b = p; break;
  case 2: r = p; g = v; b = t; break;
  case 3: r = p; g = q; b = v; break;
  case 4: r = t; g = p; b = v; break;
  case 5: r = v; g = p; b = q; break;
  }

  return Color(r, g, b);
}
