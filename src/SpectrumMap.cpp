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

  previousCursorUpdateTimeSecs = 0.0f;
  activationPatternOutdated = false;

  if(spectrumMapParameters.adaptationStrategy == SpectrumMapParameters::ErrorDriven) {
    errorLevel = spectrumMapParameters.errorThresholdHigh;
    errorLevelSmoother.setResponseFactor(1000 * audioParameters.bufferSize
					 / audioParameters.sampleRate / spectrumMapParameters.errorIntegrationTimeMs);
  }
  else
    errorLevel = 0;
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

SpectrumMapParameters SpectrumMap::getSpectrumMapParameters() const {
  return spectrumMapParameters;
}

void SpectrumMap::createSom() {
  som = new SOM(spectrumResolution, topology);
  currentActivationPattern = som->createActivationPattern();
  nextActivationPattern = som->createActivationPattern();
  resetAdaptation();
}

void SpectrumMap::resetAdaptation() {
  float somInitialValueMin, somInitialValueMax;
  switch(spectrumMapParameters.adaptationStrategy) {
  case SpectrumMapParameters::ErrorDriven:
    somInitialValueMin = spectrumMapParameters.errorThresholdHigh -
      0.001 * (spectrumMapParameters.errorThresholdHigh - spectrumMapParameters.errorThresholdLow);
    somInitialValueMax = spectrumMapParameters.errorThresholdHigh;
    break;
  case SpectrumMapParameters::TimeBased:
  default:
    somInitialValueMin = 0.0;
    somInitialValueMax = 0.0001;
    break;
  }
  som->setRandomModelValues(somInitialValueMin, somInitialValueMax);
  elapsedTimeSecs = 0.0f;
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
  feedSpectrumToSom(spectrumBinValues);
  elapsedTimeSecs += (float) numFrames / audioParameters.sampleRate;
  activationPatternOutdated = true;
}

void SpectrumMap::feedSpectrumToSom(const float *spectrum) {
  spectrumToSomInput(spectrum);
  som->train(somInput);
  som->getLastOutput(somOutput);
  if(spectrumMapParameters.adaptationStrategy == SpectrumMapParameters::ErrorDriven)
    errorLevel = errorLevelSmoother.smooth(getErrorMax());
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

float SpectrumMap::getErrorLevel() const {
  return errorLevel;
}

float SpectrumMap::getAdaptationTimeSecs() const {
  return adaptationTimeSecs;
}

float SpectrumMap::getNeighbourhoodParameter() const {
  return neighbourhoodParameter;
}

void SpectrumMap::setTrainingParameters(unsigned long numFrames) {
  switch(spectrumMapParameters.adaptationStrategy) {
  case SpectrumMapParameters::ErrorDriven:
    setErrorDrivenAdaptationValues();
    break;
  case SpectrumMapParameters::TimeBased:
    setTimeBasedAdaptationValues();
    break;
  }

  float learningParameter = getLearningParameter(adaptationTimeSecs, numFrames);
  som->setNeighbourhoodParameter(neighbourhoodParameter);
  som->setLearningParameter(learningParameter);
}

void SpectrumMap::setTimeBasedAdaptationValues() {
  if(elapsedTimeSecs < spectrumMapParameters.initialTrainingLengthSecs) {
    float relativeInitiality = 1.0f - (elapsedTimeSecs / spectrumMapParameters.initialTrainingLengthSecs);
    neighbourhoodParameter = spectrumMapParameters.normalNeighbourhoodParameter +
      (spectrumMapParameters.initialNeighbourhoodParameter - spectrumMapParameters.normalNeighbourhoodParameter) * relativeInitiality;
    adaptationTimeSecs = spectrumMapParameters.normalAdaptationTimeSecs +
      (spectrumMapParameters.initialAdaptationTimeSecs - spectrumMapParameters.normalAdaptationTimeSecs) * relativeInitiality;
  }
  else {
    neighbourhoodParameter = spectrumMapParameters.normalNeighbourhoodParameter;
    adaptationTimeSecs = spectrumMapParameters.normalAdaptationTimeSecs;
  }
}

void SpectrumMap::setErrorDrivenAdaptationValues() {
  float relativeError = (errorLevel - spectrumMapParameters.errorThresholdLow)
    / (spectrumMapParameters.errorThresholdHigh - spectrumMapParameters.errorThresholdLow);
  relativeError = clamp(relativeError, 0, 1);
  neighbourhoodParameter = spectrumMapParameters.neighbourhoodParameterMin +
    (1.0f - spectrumMapParameters.neighbourhoodParameterMin) *
    pow(relativeError, spectrumMapParameters.neighbourhoodPlasticity);
  adaptationTimeSecs = 1.0f / (spectrumMapParameters.adaptationPlasticity * errorLevel);
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

float SpectrumMap::clamp(float in, float min, float max) const {
  if(in < min)
    return min;
  if(in > max)
    return max;
  return in;
}

void SpectrumMap::writeActivationPattern(ofstream &f) {
  const SOM::ActivationPattern *activationPattern = getActivationPattern();
  SOM::ActivationPattern::const_iterator activationPatternIterator =
    activationPattern->begin();
  float v;
  for(unsigned int i = 0; i < topology->getNumNodes(); i++) {
    v = *activationPatternIterator++;
    f << v << endl;
  }
}

