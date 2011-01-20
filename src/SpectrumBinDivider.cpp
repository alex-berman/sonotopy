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

#include "SpectrumBinDivider.hpp"
#include <string.h>

using namespace sonotopy;

SpectrumBinDivider::SpectrumBinDivider(
    int _sampleRate,
    unsigned int _spectrumResolution,
    const std::vector<BinDefinition> &_binDefinitions,
    float integrationTimeMs) {
  sampleRate = _sampleRate;
  spectrumResolution = _spectrumResolution;
  binDefinitions = _binDefinitions;
  if(binDefinitions.size() == 0)
    setDefaultBins();

  nyquistFrequency = sampleRate / 2;

  setIntegrationTimeMs(integrationTimeMs);

  createBins();
  createSpectrumPositions();
}

SpectrumBinDivider::~SpectrumBinDivider() {
  delete [] spectrumPositions;
  delete [] binValues;
  delete [] bins;
}

void SpectrumBinDivider::setDefaultBins() {
  binDefinitions.clear();
  addBinDefinition(125, 50);
  addBinDefinition(150, 50);
  addBinDefinition(175, 50);
  addBinDefinition(200, 50);
  addBinDefinition(225, 50);
  addBinDefinition(250, 50);
  addBinDefinition(275, 50);
  addBinDefinition(300, 75);
  addBinDefinition(350, 100);
  addBinDefinition(400, 200);
  addBinDefinition(500, 200);
  addBinDefinition(600, 200);
  addBinDefinition(700, 200);
  addBinDefinition(800, 200);
  addBinDefinition(900, 200);
  addBinDefinition(1000, 200);
  addBinDefinition(1100, 200);
  addBinDefinition(1200, 200);
  addBinDefinition(1300, 200);
  addBinDefinition(1400, 200);
  addBinDefinition(1500, 200);
  addBinDefinition(1600, 200);
  addBinDefinition(1800, 300);
  addBinDefinition(2000, 400);
  addBinDefinition(2200, 200);
  addBinDefinition(2600, 600);
  addBinDefinition(3000, 1000);
  addBinDefinition(4000, 2000);
  addBinDefinition(5000, 2000);
  addBinDefinition(6000, 2000);
  addBinDefinition(7000, 2000);
  addBinDefinition(8000, 2000);
  addBinDefinition(9000, 2000);
  addBinDefinition(10000, 2000);
  addBinDefinition(13000, 5000);
  addBinDefinition(18000, 5000);
}

void SpectrumBinDivider::addBinDefinition(float centerFreq, float bandWidth) {
  BinDefinition binDefinition;
  binDefinition.centerFreqHz = centerFreq;
  binDefinition.bandWidthHz = bandWidth;
  binDefinitions.push_back(binDefinition);
}

void SpectrumBinDivider::createBins() {
  numBins = (unsigned int) binDefinitions.size();
  binValues = new float [numBins];
  memset(binValues, 0, sizeof(float) * numBins);

  float *valuePtr = binValues;
  bins = new Bin [numBins];
  Bin *binPtr = bins;
  std::vector<BinDefinition>::iterator binDefIterator = binDefinitions.begin();
  for(unsigned int i = 0; i < numBins; i++) {
    binPtr->definition = *binDefIterator;
    binPtr->valuePtr = valuePtr;
    valuePtr++;
    binPtr++;
    binDefIterator++;
  }
}

void SpectrumBinDivider::createSpectrumPositions() {
  spectrumPositions = new SpectrumPosition [spectrumResolution];

  Bin *binPtr = bins;
  for(unsigned int binIndex = 0; binIndex < numBins; binIndex++) {
    float halfBandWidthHz = binPtr->definition.bandWidthHz / 2;
    float freqLow = binPtr->definition.centerFreqHz - halfBandWidthHz;
    float freqHigh = binPtr->definition.centerFreqHz + halfBandWidthHz;
    int posLow = frequencyToSpectrumPosition(freqLow);
    int posHigh = frequencyToSpectrumPosition(freqHigh);
    //printf("bin %d: freq %f-%f pos %d-%d\n", binIndex, freqLow, freqHigh, posLow, posHigh);
    binPtr->size = 0;
    if(posHigh > posLow) {
      BinConnection binConnection;
      binConnection.targetBinIndex = binIndex;
      binConnection.targetBin = binPtr;
      for(int pos = posLow; pos < posHigh; pos++) {
        float freq = spectrumPositionToFrequency(pos);
        if(freq < binPtr->definition.centerFreqHz)
          binConnection.strength = (freq - freqLow) / halfBandWidthHz;
        else
          binConnection.strength = 1 - (freq - binPtr->definition.centerFreqHz) / halfBandWidthHz;
        if(binConnection.strength > 0) {
          if(binConnection.strength > 1)
            binConnection.strength = 1;
          //printf("  pos %d strength %f\n", pos, link.strength);
          spectrumPositions[pos].binConnections.push_back(binConnection);
          binPtr->size += binConnection.strength;
        }
      }
    }
    binPtr++;
  }
}

void SpectrumBinDivider::setIntegrationTimeMs(float _integrationTimeMs) {
  integrationTimeMs = _integrationTimeMs;
}

void SpectrumBinDivider::feedSpectrum(const float *spectrum, unsigned long numFrames) {
  clearBinPowerSums();
  accummulateBinPowerSumsFromSpectrum(spectrum);
  calculateBinValues(numFrames);
}

void SpectrumBinDivider::clearBinPowerSums() {
  Bin *binPtr = bins;
  for(unsigned int binIndex = 0; binIndex < numBins; binIndex++) {
    binPtr->powerSum = 0;
    binPtr++;
  }
}

void SpectrumBinDivider::accummulateBinPowerSumsFromSpectrum(const float *spectrum) {
  const float *spectrumPtr = spectrum;
  float power;
  SpectrumPosition *spectrumPositionPtr = spectrumPositions;
  std::vector<BinConnection>::iterator binConnectionsBegin, binConnectionsEnd, c;
  Bin *targetBin;

  for(unsigned int i = 0; i < spectrumResolution; i++) {
    power = *spectrumPtr;
    binConnectionsBegin = spectrumPositionPtr->binConnections.begin();
    binConnectionsEnd = spectrumPositionPtr->binConnections.end();
    for(c = binConnectionsBegin; c != binConnectionsEnd; c++) {
      targetBin = c->targetBin;
      targetBin->powerSum += power * c->strength;
    }
    spectrumPositionPtr++;
    spectrumPtr++;
  }
}

void SpectrumBinDivider::calculateBinValues(unsigned long numFrames) {
  static float integrationFactor;
  static float targetValue;

  integrationFactor = getIntegrationFactor(numFrames);
  Bin *binPtr = bins;
  for(unsigned int binIndex = 0; binIndex < numBins; binIndex++) {
    if(binPtr->size > 0) {
      targetValue = binPtr->powerSum / binPtr->size;
      *(binPtr->valuePtr) += integrationFactor * (targetValue - *(binPtr->valuePtr));
    }
    binPtr++;
  }
}

float SpectrumBinDivider::getIntegrationFactor(unsigned long numFrames) {
  static float integrationFactor;
  if(integrationTimeMs <= 0) {
    return 1;
  }
  else {
    integrationFactor = 1000 * numFrames / sampleRate / integrationTimeMs;
    if(integrationFactor >= 1)
      return 1;
    else
      return integrationFactor;
  }
}

int SpectrumBinDivider::frequencyToSpectrumPosition(float freq) {
  // warning: this function keeps the result within legal boundaries (0 to spectrumResolution-1)
  int pos = (int) (spectrumResolution * freq / nyquistFrequency);
  if(pos < 0)
    pos = 0;
  else if(pos >= (int) (spectrumResolution-1))
    pos = (int) spectrumResolution - 1;
  return pos;
}

float SpectrumBinDivider::spectrumPositionToFrequency(int pos) {
  return (float) pos * nyquistFrequency / spectrumResolution;
}
