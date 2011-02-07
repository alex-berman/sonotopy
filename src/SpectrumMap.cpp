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

#include "SpectrumMap.hpp"
#include <math.h>

using namespace sonotopy;
using namespace std;

SpectrumMap::SpectrumMap(Topology *_topology,
			 const AudioParameters &_audioParameters,
			 const SpectrumMapParameters &_spectrumMapParameters)
{
  topology = _topology;
  audioParameters = _audioParameters;
  spectrumMapParameters = _spectrumMapParameters;

  createSpectrumAnalyzer();
  createSpectrumBinDivider();
  createSom();
  createSomInput();
  createSomOutput();

  elapsedTimeSecs = 0.0f;
  previousCursorUpdateTimeSecs = 0.0f;
  activationPatternOutdated = false;
  errorLevel = 0.001; // TODO: parametrsize?
  errorLevelSmoother.setResponseFactor(100.0f / audioParameters.bufferSize); // TODO: parametrsize
}

SpectrumMap::~SpectrumMap() {
  delete som;
  delete spectrumBinDivider;
  delete spectrumAnalyzer;
  delete currentActivationPattern;
  delete nextActivationPattern;
}

Topology* SpectrumMap::getTopology() const {
  return topology;
}

void SpectrumMap::createSom() {
  som = new SOM(spectrumResolution, topology);
  som->setRandomModelValues(0.0, 0.0001);
  currentActivationPattern = som->createActivationPattern();
  nextActivationPattern = som->createActivationPattern();
}

void SpectrumMap::createSomInput() {
  for(int i = 0; i < spectrumResolution; i++)
    somInput.push_back(0);
}

void SpectrumMap::createSomOutput() {
  for(unsigned int i = 0; i < topology->getNumNodes(); i++)
    somOutput.push_back(0);
}

const SOM::ActivationPattern* SpectrumMap::getActivationPattern() {
  if(activationPatternOutdated) {
    som->getActivationPattern(nextActivationPattern);
    *currentActivationPattern = *nextActivationPattern;
    activationPatternOutdated = false;
  }
  return currentActivationPattern;
}

void SpectrumMap::createSpectrumAnalyzer() {
  spectrumAnalyzer = new SpectrumAnalyzer();
}

void SpectrumMap::createSpectrumBinDivider() {
  spectrumBinDivider = new SpectrumBinDivider(audioParameters.sampleRate,
                                              spectrumAnalyzer->getSpectrumResolution());
  spectrumResolution = spectrumBinDivider->getNumBins();
}

void SpectrumMap::feedAudio(const float *audio, unsigned long numFrames) {
  spectrumAnalyzer->feedAudioFrames(audio, numFrames);
  spectrum = spectrumAnalyzer->getSpectrum();
  spectrumBinDivider->feedSpectrum(spectrum, numFrames);
  spectrumBinValues = spectrumBinDivider->getBinValues();
  setTrainingParameters(numFrames);
  feedSpectrumToSom(spectrumBinValues, spectrumMapParameters.enableLiveTraining);
  elapsedTimeSecs += (float) numFrames / audioParameters.sampleRate;
  activationPatternOutdated = true;
}

void SpectrumMap::feedSpectrumToSom(const float *spectrum, bool train) {
  spectrumToSomInput(spectrum);
  if(train) {
    som->train(somInput);
    som->getLastOutput(somOutput);
    errorLevel = errorLevelSmoother.smooth(getErrorMax());
  }
  else {
    som->getOutput(somInput, somOutput);
  }
}

void SpectrumMap::spectrumToSomInput(const float *spectrum) {
  SOM::Sample::iterator somInputPtr = somInput.begin();
  const float *spectrumPtr = spectrum;
  for(int i = 0; i < spectrumResolution; i++) {
    *somInputPtr++ = (double) *spectrumPtr++;
  }
}

int SpectrumMap::getWinnerId() const {
  return som->getLastWinner();
}

void SpectrumMap::setTrainingParameters(unsigned long numFrames) {
  float neighbourhoodParameter;
  if(errorLevel < spectrumMapParameters.errorThreshold)
    neighbourhoodParameter = spectrumMapParameters.neighbourhoodParameterMin;
  else
    neighbourhoodParameter = spectrumMapParameters.neighbourhoodParameterMin +
      pow(errorLevel - spectrumMapParameters.errorThreshold, spectrumMapParameters.neighbourhoodPlasticity);
  float adaptationTimeSecs =
    1.0f / (spectrumMapParameters.adaptationPlasticity * errorLevel);
  float learningParameter = getLearningParameter(adaptationTimeSecs, numFrames);
  som->setNeighbourhoodParameter(neighbourhoodParameter);
  som->setLearningParameter(learningParameter);
  // TEMP:
  if(((int) (elapsedTimeSecs * 1000)) % 10 == 0)
    printf("errorLevel=%.5f adaptationTimeSecs=%.5f neighbourhoodParameter=%.5f\n", errorLevel, adaptationTimeSecs, neighbourhoodParameter);
}

float SpectrumMap::getLearningParameter(float adaptationTimeSecs, unsigned long numFrames) {
  float learningParameter = (float) numFrames / audioParameters.sampleRate / adaptationTimeSecs;
  if(learningParameter >= 1)
    return 1;
  else
    return learningParameter;
}

void SpectrumMap::setSpectrumIntegrationTimeMs(float integrationTimeMs) {
  spectrumBinDivider->setIntegrationTimeMs(integrationTimeMs);
}

void SpectrumMap::moveTopologyCursorTowardsWinner() {
  int winnerId = getWinnerId();
  if(previousCursorUpdateTimeSecs <= 0.0f) {
    topology->placeCursorAtNode(winnerId);
  }
  else {
    if(spectrumMapParameters.trajectorySmoothness > 0) {
      float deltaSecs = elapsedTimeSecs - previousCursorUpdateTimeSecs;
      float amount = deltaSecs / spectrumMapParameters.trajectorySmoothness;
      if(amount > 1)
	topology->placeCursorAtNode(winnerId);
      else
	topology->moveCursorTowardsNode(winnerId, amount);
    }
    else {
      topology->placeCursorAtNode(winnerId);
    }
  }
  previousCursorUpdateTimeSecs = elapsedTimeSecs;
}

float SpectrumMap::getErrorMin() const {
  return som->getOutputMin();
}

float SpectrumMap::getErrorMax() const {
  return som->getOutputMax();
}
