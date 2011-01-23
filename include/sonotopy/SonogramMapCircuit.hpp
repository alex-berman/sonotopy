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

#ifndef _SonogramMapCircuit_hpp_
#define _SonogramMapCircuit_hpp_

#include "AudioParameters.hpp"
#include "SonogramMapCircuitParameters.hpp"
#include "SonogramMap.hpp"
#include "SpectrumAnalyzer.hpp"
#include "SpectrumBinDivider.hpp"
#include "Sonogram.hpp"
#include "SOM.hpp"

namespace sonotopy {

class SonogramMapCircuit {
public:
  SonogramMapCircuit(Topology *, const AudioParameters &, const SonogramMapCircuitParameters &);
  ~SonogramMapCircuit();
  void feedAudio(const float *audio, unsigned long numFrames);
  const SonogramMap *getSonogramMap() { return sonogramMap; }
  const SonogramMap::ActivationPattern* getActivationPattern();
  const SpectrumAnalyzer* getSpectrumAnalyzer() { return spectrumAnalyzer; }
  const Sonogram* getSonogram() { return sonogram; }
  const SpectrumBinDivider* getSpectrumBinDivider() { return spectrumBinDivider; }
  void setSpectrumIntegrationTimeMs(float);

protected:
  void createSonogramMap();
  void createSpectrumAnalyzer();
  void createSpectrumBinDivider();
  void createSonogram();
  void createSOM();
  void setSonogramMapTrainingParameters(unsigned long numFrames);
  float getLearningParameter(float adaptationTimeSecs, unsigned long numFrames);

  Topology *topology;
  AudioParameters audioParameters;
  SonogramMapCircuitParameters sonogramMapCircuitParameters;
  SonogramMap *sonogramMap;
  SonogramMap::ActivationPattern *activationPattern;
  SpectrumAnalyzer *spectrumAnalyzer;
  SpectrumBinDivider *spectrumBinDivider;
  Sonogram *sonogram;
  const float *spectrum;
  const float *spectrumBinValues;
  float elapsedTimeSecs;
};

}

#endif
