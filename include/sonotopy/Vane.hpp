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

#ifndef _VANE_HPP_
#define _VANE_HPP_

#include "AudioParameters.hpp"
#include "SonogramMapCircuitParameters.hpp"
#include "CircleMapCircuitParameters.hpp"
#include "CircleMapCircuit.hpp"
#include "CircleTopology.hpp"

namespace sonotopy {

class Vane {
public:
  Vane(const AudioParameters &, const CircleMapCircuitParameters &params=CircleMapCircuitParameters());
  ~Vane();
  void feedAudio(const float *audio, unsigned long numFrames);
  float getAngle() const;
  void setResponseTimeMs(float);

private:
  void createCircleMapCircuit();
  void calculateAngle();
  void rotateTowards(float angle);

  Topology *topology;
  CircleMapCircuit *circleMapCircuit;
  AudioParameters audioParameters;
  CircleMapCircuitParameters circleMapCircuitParameters;
  const SonogramMap *circleMap;
  CircleTopology *circleTopology;
  const SpectrumBinDivider *spectrumBinDivider;
  float angle;
  float responseTimeMs;
  float responseFactor;
};

}

#endif
