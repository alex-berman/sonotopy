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

#include "Vane.hpp"
#include <math.h>

using namespace sonotopy;

Vane::Vane(const AudioParameters &_audioParameters, const CircleMapCircuitParameters &_circleMapCircuitParameters) {
  audioParameters = _audioParameters;
  circleMapCircuitParameters = _circleMapCircuitParameters;
  createCircleMapCircuit();
}

Vane::~Vane() {
  delete circleMapCircuit;
}

void Vane::createCircleMapCircuit() {
  circleMapCircuit = new CircleMapCircuit(audioParameters, circleMapCircuitParameters);
}

void Vane::feedAudio(const float *audio, unsigned long numFrames) {
  circleMapCircuit->feedAudio(audio, numFrames);
}

float Vane::getAngle() const {
  return circleMapCircuit->getWinnerAngle();
}
