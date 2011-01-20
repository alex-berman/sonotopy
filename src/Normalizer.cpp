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

#include "Normalizer.hpp"

using namespace sonotopy;

Normalizer::Normalizer() {
  max = 0.0f;
  setAdaptationFactor(1.0f);
}

float Normalizer::normalize(float value) {
  if(value > max) {
    max = value;
    return 1.0f;
  }
  else if(max == 0.0f)
    return 0.0f;
  else {
    float result = value / max;
    if(adaptationFactor < 1.0f)
      max -= (max - value) * adaptationFactor;
    return result;
  }
}

void Normalizer::setAdaptationFactor(float _adaptationFactor) {
  adaptationFactor = _adaptationFactor;
}
