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

#include "SpectrumMapCircuit.hpp"

using namespace sonotopy;

SpectrumMapCircuit::SpectrumMapCircuit(Topology *_topology,
                                       const AudioParameters &_audioParameters,
                                       const SpectrumMapCircuitParameters &_spectrumMapCircuitParameters)
{
  topology = _topology;
  audioParameters = _audioParameters;
  spectrumMapCircuitParameters = _spectrumMapCircuitParameters;

  createSpectrumAnalyzer();
  createSpectrumBinDivider();
  createSpectrumMap();
  elapsedTimeSecs = 0.0f;
  activationPatternOutdated = false;
}

SpectrumMapCircuit::~SpectrumMapCircuit() {
  delete spectrumBinDivider;
  delete spectrumAnalyzer;
  delete spectrumMap;
  delete currentActivationPattern;
  delete nextActivationPattern;
}

const SpectrumMap::ActivationPattern* SpectrumMapCircuit::getActivationPattern() {
  if(activationPatternOutdated) {
    spectrumMap->getActivationPattern(nextActivationPattern);
    *currentActivationPattern = *nextActivationPattern;
    activationPatternOutdated = false;
  }
  return currentActivationPattern;
}

void SpectrumMapCircuit::createSpectrumAnalyzer() {
  spectrumAnalyzer = new SpectrumAnalyzer();
}

void SpectrumMapCircuit::createSpectrumBinDivider() {
  spectrumBinDivider = new SpectrumBinDivider(audioParameters.sampleRate,
                                              spectrumAnalyzer->getSpectrumResolution());
}

void SpectrumMapCircuit::createSpectrumMap() {
  spectrumMap = new SpectrumMap(topology, spectrumBinDivider->getNumBins());
  currentActivationPattern = spectrumMap->createActivationPattern();
  nextActivationPattern = spectrumMap->createActivationPattern();
}

void SpectrumMapCircuit::feedAudio(const float *audio, unsigned long numFrames) {
  spectrumAnalyzer->feedAudioFrames(audio, numFrames);
  spectrum = spectrumAnalyzer->getSpectrum();
  spectrumBinDivider->feedSpectrum(spectrum, numFrames);
  spectrumBinValues = spectrumBinDivider->getBinValues();
  setSpectrumMapTrainingParameters(numFrames);
  spectrumMap->feedSpectrum(spectrumBinValues, spectrumMapCircuitParameters.enableLiveTraining);
  elapsedTimeSecs += (float) numFrames / audioParameters.sampleRate;
  activationPatternOutdated = true;
}

int SpectrumMapCircuit::getLastWinner() const {
  return spectrumMap->getLastWinner();
}

void SpectrumMapCircuit::setSpectrumMapTrainingParameters(unsigned long numFrames) {
  float neighbourhoodParameter;
  float adaptationTimeSecs;
  float learningParameter;
  if(elapsedTimeSecs < spectrumMapCircuitParameters.initialTrainingLengthSecs) {
    float relativeInitiality = 1.0f - (elapsedTimeSecs / spectrumMapCircuitParameters.initialTrainingLengthSecs);
    neighbourhoodParameter = spectrumMapCircuitParameters.normalNeighbourhoodParameter +
      (spectrumMapCircuitParameters.initialNeighbourhoodParameter - spectrumMapCircuitParameters.normalNeighbourhoodParameter) * relativeInitiality;
    adaptationTimeSecs = spectrumMapCircuitParameters.normalAdaptationTimeSecs +
      (spectrumMapCircuitParameters.initialAdaptationTimeSecs - spectrumMapCircuitParameters.normalAdaptationTimeSecs) * relativeInitiality;
  }
  else {
    neighbourhoodParameter = spectrumMapCircuitParameters.normalNeighbourhoodParameter;
    adaptationTimeSecs = spectrumMapCircuitParameters.normalAdaptationTimeSecs;
  }
  learningParameter = getLearningParameter(adaptationTimeSecs, numFrames);
  spectrumMap->setNeighbourhoodParameter(neighbourhoodParameter);
  spectrumMap->setLearningParameter(learningParameter);
}

float SpectrumMapCircuit::getLearningParameter(float adaptationTimeSecs, unsigned long numFrames) {
  float learningParameter = (float) numFrames / audioParameters.sampleRate / adaptationTimeSecs;
  if(learningParameter >= 1)
    return 1;
  else
    return learningParameter;
}

void SpectrumMapCircuit::setSpectrumIntegrationTimeMs(float integrationTimeMs) {
  spectrumBinDivider->setIntegrationTimeMs(integrationTimeMs);
}
