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

#include "SonogramMapCircuit.hpp"

using namespace sonotopy;

SonogramMapCircuit::SonogramMapCircuit(Topology *_topology,
                                       const AudioParameters &_audioParameters,
                                       const SonogramMapCircuitParameters &_sonogramMapCircuitParameters)
{
  topology = _topology;
  audioParameters = _audioParameters;
  sonogramMapCircuitParameters = _sonogramMapCircuitParameters;

  createSpectrumAnalyzer();
  createSpectrumBinDivider();
  createSonogram();
  createSonogramMap();
  elapsedTimeSecs = 0.0f;
}

SonogramMapCircuit::~SonogramMapCircuit() {
  delete spectrumBinDivider;
  delete spectrumAnalyzer;
  delete sonogram;
  delete sonogramMap;
  delete activationPattern;
}

void SonogramMapCircuit::createSpectrumAnalyzer() {
  spectrumAnalyzer = new SpectrumAnalyzer();
}

void SonogramMapCircuit::createSpectrumBinDivider() {
  spectrumBinDivider = new SpectrumBinDivider(audioParameters.sampleRate,
                                              spectrumAnalyzer->getSpectrumResolution());
}

void SonogramMapCircuit::createSonogram() {
  sonogram = new Sonogram(sonogramMapCircuitParameters.sonogramHistoryLength,
                          spectrumBinDivider->getNumBins());
}

void SonogramMapCircuit::createSonogramMap() {
  sonogramMap = new SonogramMap(topology,
                                sonogramMapCircuitParameters.sonogramHistoryLength,
                                spectrumBinDivider->getNumBins());
  activationPattern = sonogramMap->createActivationPattern();
}

void SonogramMapCircuit::feedAudio(const float *audio, unsigned long numFrames) {
  spectrumAnalyzer->feedAudioFrames(audio, numFrames);
  spectrum = spectrumAnalyzer->getSpectrum();
  spectrumBinDivider->feedSpectrum(spectrum, numFrames);
  spectrumBinValues = spectrumBinDivider->getBinValues();
  sonogram->feedSpectrum(spectrumBinValues);
  setSonogramMapTrainingParameters(numFrames);
  sonogramMap->feedSonogram(sonogram, sonogramMapCircuitParameters.enableLiveTraining);
  elapsedTimeSecs += (float) numFrames / audioParameters.sampleRate;
}

void SonogramMapCircuit::setSonogramMapTrainingParameters(unsigned long numFrames) {
  float neighbourhoodParameter;
  float adaptationTimeSecs;
  float learningParameter;
  if(elapsedTimeSecs < sonogramMapCircuitParameters.initialTrainingLengthSecs) {
    float relativeInitiality = 1.0f - (elapsedTimeSecs / sonogramMapCircuitParameters.initialTrainingLengthSecs);
    neighbourhoodParameter = sonogramMapCircuitParameters.normalNeighbourhoodParameter +
      (sonogramMapCircuitParameters.initialNeighbourhoodParameter - sonogramMapCircuitParameters.normalNeighbourhoodParameter) * relativeInitiality;
    adaptationTimeSecs = sonogramMapCircuitParameters.normalAdaptationTimeSecs +
      (sonogramMapCircuitParameters.initialAdaptationTimeSecs - sonogramMapCircuitParameters.normalAdaptationTimeSecs) * relativeInitiality;
  }
  else {
    neighbourhoodParameter = sonogramMapCircuitParameters.normalNeighbourhoodParameter;
    adaptationTimeSecs = sonogramMapCircuitParameters.normalAdaptationTimeSecs;
  }
  learningParameter = getLearningParameter(adaptationTimeSecs, numFrames);
  sonogramMap->setNeighbourhoodParameter(neighbourhoodParameter);
  sonogramMap->setLearningParameter(learningParameter);
}

float SonogramMapCircuit::getLearningParameter(float adaptationTimeSecs, unsigned long numFrames) {
  float learningParameter = (float) numFrames / audioParameters.sampleRate / adaptationTimeSecs;
  if(learningParameter >= 1)
    return 1;
  else
    return learningParameter;
}

void SonogramMapCircuit::setSpectrumIntegrationTimeMs(float integrationTimeMs) {
  spectrumBinDivider->setIntegrationTimeMs(integrationTimeMs);
}
