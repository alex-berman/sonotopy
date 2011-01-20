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
  setResponseTimeMs(0.1);
  angle = 0;
}

Vane::~Vane() {
  delete circleMapCircuit;
}

void Vane::createCircleMapCircuit() {
  circleMapCircuit = new CircleMapCircuit(audioParameters, circleMapCircuitParameters);
  spectrumBinDivider = circleMapCircuit->getSpectrumBinDivider();
  circleMap = circleMapCircuit->getSonogramMap();
  circleTopology = (CircleTopology*) circleMap->getTopology();
}

void Vane::feedAudio(const float *audio, unsigned long numFrames) {
  circleMapCircuit->feedAudio(audio, numFrames);
  calculateAngle();
}

void Vane::calculateAngle() {
  int winnerId = circleMap->getLastWinner();
  CircleTopology::Node node = circleTopology->getNode(winnerId);
  rotateTowards(node.angle);
}

void Vane::setResponseTimeMs(float _responseTimeMs) {
  responseTimeMs = _responseTimeMs;
  if(responseTimeMs > 0) {
    responseFactor = 1000 * audioParameters.bufferSize / audioParameters.sampleRate / responseTimeMs;
    if(responseFactor >= 1)
      responseFactor = 1;
  }
}

float Vane::getAngle() const { return angle; }

void Vane::rotateTowards(float targetAngle) {
  if(responseTimeMs <= 0) {
    angle = targetAngle;
  }
  else {
    if(fabsf(angle - targetAngle) < M_PI)
      angle += (targetAngle - angle) * responseFactor;
    else
      angle -= (2 * M_PI - targetAngle + angle) * responseFactor;
  }
}
