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

#include "Smoother.hpp"

using namespace sonotopy;

Smoother::Smoother() {
  initialized = false;
}

float Smoother::smooth(float newValue) {
  if(initialized)
    currentValue += (newValue - currentValue) * responseFactor;
  else {
    currentValue = newValue;
    initialized = true;
  }
  return currentValue;
}

void Smoother::setResponseFactor(float _responseFactor) {
  responseFactor = _responseFactor;
  if(responseFactor > 1)
    responseFactor = 1;
}

float Smoother::getValue() {
  return currentValue;
}
