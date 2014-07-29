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

#ifndef _SpectrumBinDivider_hpp_
#define _SpectrumBinDivider_hpp_

#include <vector>

namespace sonotopy {

typedef struct {
  float centerFreqHz;
  float bandWidthHz;
} BinDefinition;


class SpectrumBinDivider {
public:
  SpectrumBinDivider(int sampleRate,
                     unsigned int spectrumResolution,
                     const std::vector<sonotopy::BinDefinition> &_binDefinitions = std::vector<sonotopy::BinDefinition>(),
                     float integrationTimeMs = 40.0f);
  ~SpectrumBinDivider();
  int getSampleRate() { return sampleRate; }
  void feedSpectrum(const float *spectrum, unsigned long numFrames);
  float *getBinValues() const { return binValues; }
  unsigned int getNumBins() const { return numBins; }
  void setIntegrationTimeMs(float);
  float getIntegrationTimeMs() const { return integrationTimeMs; }

private:
  typedef struct {
    sonotopy::BinDefinition definition;
    float size; // integral of band envelope = sum of strengths of the connections to this bin
    float powerSum;
    float *valuePtr;
  } Bin;

  /* spectrumPositions has size halfWindowSize. each position contains connections to bins.
     each connection has a strength determined by the distance to the bin's center frequency.
     some portions of the spectrum may have no bin connections and other several. */
  typedef struct {
    unsigned int targetBinIndex;
    Bin *targetBin;
    float strength;
  } BinConnection;

  typedef struct {
    std::vector<BinConnection> binConnections;
  } SpectrumPosition;
  SpectrumPosition *spectrumPositions;

  int sampleRate;
  unsigned int spectrumResolution;
  std::vector<sonotopy::BinDefinition> binDefinitions;
  int nyquistFrequency;
  Bin *bins;
  float *binValues; // this is where the output is built
  unsigned int numBins;
  float integrationTimeMs;

  void createBins();
  void createSpectrumPositions();
  void clearBinPowerSums();
  void accummulateBinPowerSumsFromSpectrum(const float *spectrum);
  void calculateBinValues(unsigned long numFrames);
  int frequencyToSpectrumPosition(float freq);
  float spectrumPositionToFrequency(int pos);
  float getIntegrationFactor(unsigned long numFrames);
  void addBinDefinition(float centerFreq, float bandWidth);
  void setDefaultBins();
};

}

#endif
