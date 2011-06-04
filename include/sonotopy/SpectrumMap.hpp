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

#ifndef _SpectrumMap_hpp_
#define _SpectrumMap_hpp_

#include "AudioParameters.hpp"
#include "SpectrumMapParameters.hpp"
#include "Topology.hpp"
#include "SpectrumAnalyzer.hpp"
#include "SpectrumBinDivider.hpp"
#include "SOM.hpp"
#include "Smoother.hpp"
#include <vector>

namespace sonotopy {

class SpectrumMap {
public:
  SpectrumMap(Topology *, const AudioParameters &, const SpectrumMapParameters &);
  ~SpectrumMap();
  void feedAudio(const float *audio, unsigned long numFrames);
  int getWinnerId() const;
  const SpectrumAnalyzer* getSpectrumAnalyzer() { return spectrumAnalyzer; }
  const SpectrumBinDivider* getSpectrumBinDivider() { return spectrumBinDivider; }
  const SOM::ActivationPattern* getActivationPattern();
  Topology* getTopology() const;
  void setSpectrumIntegrationTimeMs(float);
  void moveTopologyCursorTowardsWinner();
  float getErrorMin() const;
  float getErrorMax() const;
  float getErrorLevel() const;
  float getAdaptationTimeSecs() const;
  float getNeighbourhoodParameter() const;
  SpectrumMapParameters getSpectrumMapParameters() const;
  void resetAdaptation();

protected:
  void createSpectrumAnalyzer();
  void createSpectrumBinDivider();
  void createSom();
  void createSomInput();
  void createSomOutput();
  void feedSpectrumToSom(const float *spectrum);
  void spectrumToSomInput(const float *);
  void setTrainingParameters(unsigned long numFrames);
  float getLearningParameter(float adaptationTimeSecs, unsigned long numFrames);
  void setTimeBasedAdaptationValues();
  void setErrorDrivenAdaptationValues();
  float clamp(float in, float min, float max) const;

  AudioParameters audioParameters;
  SpectrumMapParameters spectrumMapParameters;
  SOM *som;
  Topology *topology;
  int spectrumResolution;
  SOM::Sample somInput;
  SOM::Output somOutput;
  SOM::ActivationPattern *currentActivationPattern;
  SOM::ActivationPattern *nextActivationPattern;
  SpectrumAnalyzer *spectrumAnalyzer;
  SpectrumBinDivider *spectrumBinDivider;
  const float *spectrum;
  const float *spectrumBinValues;
  float elapsedTimeSecs;
  float previousCursorUpdateTimeSecs;
  bool activationPatternOutdated;
  float neighbourhoodParameter;
  float adaptationTimeSecs;
  float errorLevel;
  Smoother errorLevelSmoother;
};

}

#endif
