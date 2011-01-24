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

SpectrumMap::SpectrumMap(Topology *_topology, int _spectrumResolution)
{
  topology = _topology;
  spectrumResolution = _spectrumResolution;

  createSom();
  createSomInput();
  createSomOutput();

  setNeighbourhoodParameter(0.5);
  setLearningParameter(0.5);
}

SpectrumMap::~SpectrumMap() {
  delete som;
}

Topology* SpectrumMap::getTopology() const {
  return topology;
}

void SpectrumMap::createSom() {
  som = new SOM(spectrumResolution, topology);
  som->setRandomModelValues(0.0, 0.0001);
}

void SpectrumMap::createSomInput() {
  for(int i = 0; i < spectrumResolution; i++)
    somInput.push_back(0);
}

void SpectrumMap::createSomOutput() {
  for(unsigned int i = 0; i < topology->getNumNodes(); i++)
    somOutput.push_back(0);
}

void SpectrumMap::setNeighbourhoodParameter(float neighbourhoodParameter) {
  som->setNeighbourhoodParameter(neighbourhoodParameter);
}

void SpectrumMap::setLearningParameter(float learningParameter) {
  som->setLearningParameter(learningParameter);
}

void SpectrumMap::feedSpectrum(const float *spectrum, bool train) {
  spectrumToSomInput(spectrum);
  if(train) {
    som->train(somInput);
    som->getLastOutput(somOutput);
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

int SpectrumMap::getLastWinner() const {
  return som->getLastWinner();
}

SpectrumMap::ActivationPattern* SpectrumMap::createActivationPattern() const {
  vector<float> *pattern = new vector<float>();
  for(unsigned int i = 0; i < topology->getNumNodes(); i++)
    pattern->push_back(0);
  return pattern;
}

void SpectrumMap::getActivationPattern(ActivationPattern *activationPattern) const {
  float min, max, range;
  getMinAndMax(somOutput, min, max);
  range = max - min;
  if(range > 0) {
    ActivationPattern::iterator activationPatternNode = activationPattern->begin();
    for(SOM::Output::const_iterator somOutputNode = somOutput.begin();
	somOutputNode != somOutput.end(); somOutputNode++) {
      *activationPatternNode = ((float)*somOutputNode - min) / range;
      activationPatternNode++;
    }
  }
  else {
    fill(activationPattern->begin(), activationPattern->end(), 0);
  }
}

void SpectrumMap::getMinAndMax(const vector<double> &values, float &min, float &max) const {
  float value;
  vector<double>::const_iterator i = values.begin();
  min = max = (float) *i;
  for(; i != values.end(); i++) {
    value = (float) *i;
    if(value < min)
      min = value;
    else if(value > max)
      max = value;
  }
}
