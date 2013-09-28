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

#include "Color.hpp"

#ifndef _ColorScheme_hpp_
#define _ColorScheme_hpp_

class ColorScheme {
public:
  virtual Color getColor(float) = 0;
  Color HSV_to_RGB(float, float, float);
};

class Grayscale : public ColorScheme {
public:
  Color getColor(float);
  const static float contrast;
};

class Rainbow : public ColorScheme {
public:
  Color getColor(float);
};

#endif