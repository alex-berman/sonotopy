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

#ifndef _SpectrumMapCircuit_hpp_
#define _SpectrumMapCircuit_hpp_

#include "AudioParameters.hpp"
#include "SpectrumMapCircuitParameters.hpp"
#include "SpectrumMap.hpp"
#include "SpectrumAnalyzer.hpp"
#include "SpectrumBinDivider.hpp"
#include "SOM.hpp"

namespace sonotopy {

class SpectrumMapCircuit {
public:
  SpectrumMapCircuit(Topology *, const AudioParameters &, const SpectrumMapCircuitParameters &);
  ~SpectrumMapCircuit();
  void feedAudio(const float *audio, unsigned long numFrames);
  int getLastWinner() const;
  const SpectrumMap *getSpectrumMap() { return spectrumMap; }
  const SpectrumMap::ActivationPattern* getActivationPattern();
  const SpectrumAnalyzer* getSpectrumAnalyzer() { return spectrumAnalyzer; }
  const SpectrumBinDivider* getSpectrumBinDivider() { return spectrumBinDivider; }
  void setSpectrumIntegrationTimeMs(float);

protected:
  void createSpectrumMap();
  void createSpectrumAnalyzer();
  void createSpectrumBinDivider();
  void createSOM();
  void setSpectrumMapTrainingParameters(unsigned long numFrames);
  float getLearningParameter(float adaptationTimeSecs, unsigned long numFrames);

  Topology *topology;
  AudioParameters audioParameters;
  SpectrumMapCircuitParameters spectrumMapCircuitParameters;
  SpectrumMap *spectrumMap;
  SpectrumMap::ActivationPattern *currentActivationPattern;
  SpectrumMap::ActivationPattern *nextActivationPattern;
  SpectrumAnalyzer *spectrumAnalyzer;
  SpectrumBinDivider *spectrumBinDivider;
  const float *spectrum;
  const float *spectrumBinValues;
  float elapsedTimeSecs;

private:
  bool activationPatternOutdated;
};

}

#endif
