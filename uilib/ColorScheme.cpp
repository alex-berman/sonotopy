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
#include <string.h>
#include <math.h>
#include <exception>

using namespace std;

void ColorScheme::addParserArguments(cmdline::parser &parser) {
  parser.add<string>("colorScheme", '\0', "Color scheme", false, "grayscale",
		     cmdline::oneof<string>("grayscale", "rainbow", "stripes"));
  parser.add<float>("contrast", '\0', "Contrast (>0)", false, 5.0);
}

ColorScheme* ColorScheme::createFromParser(cmdline::parser &parser) {
  string colorSchemeName = parser.get<string>("colorScheme");
  float contrast = parser.get<float>("contrast");
  if(colorSchemeName == "grayscale")
    return new Grayscale(contrast);
  else if(colorSchemeName == "stripes")
    return new Stripes(contrast);
  else if(colorSchemeName == "rainbow")
    return new Rainbow();
  else
    throw runtime_error("unknown color scheme" + colorSchemeName);
}

Color Grayscale::getColor(float fraction) {
  float v = pow(fraction, contrast);
  return Color(v, v, v);
}


Color Stripes::getColor(float fraction) {
  float v = pow(sin(fraction * 10.0f), contrast);
  return Color(v, v, v);
}


Color Rainbow::getColor(float fraction) {
  return HSV_to_RGB(fraction, 1.0f, 1.0f);
}


Color ColorScheme::HSV_to_RGB(float h, float s, float v) {
  int i = int(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  
  switch(i % 6) {
  case 0: return Color(v, t, p);
  case 1: return Color(q, v, p);
  case 2: return Color(p, v, t);
  case 3: return Color(p, q, v);
  case 4: return Color(t, p, v);
  default: return Color(v, p, q);
  }
}
