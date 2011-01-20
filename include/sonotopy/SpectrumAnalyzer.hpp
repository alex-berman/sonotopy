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

#ifndef _SpectrumAnalyzer_hpp_
#define _SpectrumAnalyzer_hpp_

#include "CircularBuffer.hpp"
#include <fftw3.h>

namespace sonotopy {

class SpectrumAnalyzer {
public:
  typedef enum {
    dB,
    Amplitude
  } PowerScale;

  typedef enum {
    NoWindowFunction,
    BlackmanHarris
  } WindowFunction;

  SpectrumAnalyzer(int spectrumWindowSize = 16384,
                   float spectrumWindowOverlap = (float) 15/16,
                   PowerScale = SpectrumAnalyzer::Amplitude,
                   WindowFunction = SpectrumAnalyzer::BlackmanHarris);
  ~SpectrumAnalyzer();
  void feedAudioFrames(const float *input, unsigned long numFrames);
  float *getSpectrum() const { return spectrum; }
  int getWindowSize() const { return windowSize; }
  int getSpectrumResolution() const { return spectrumResolution; }
  PowerScale getPowerScale() const { return powerScale; }
  void setDecibelReference(double dB_reference);
  float *getInputWindow() const { return inputHistoryBuffer; }

private:
  typedef double (SpectrumAnalyzer::*PowerScalingFunction)(double);

  double dB_defaultReference;
  PowerScale powerScale;
  WindowFunction windowFunction;
  int windowSize;
  float windowOverlap;
  unsigned long spectrumResolution;
  CircularBuffer<float> *inputHistory;
  float *inputHistoryBuffer;
  fftw_plan fftPlan;
  fftw_complex *fftIn, *fftOut;
  float *spectrum;
  double fftOutMax;
  double dB_reference;
  double log10_min, log10_scalefactor;
  double *windowFunctionTable;
  unsigned long numUnconsumedFrames;
  unsigned long numNewFramesPerFFT;

  void appendAudioToHistory(const float *input, unsigned long numFrames);
  void processUnconsumedFrames();
  void performFFT();
  void inputHistoryToFftIn();
  void fftOutToSpectrum();
  void createBlackmanHarrisWindowFunctionTable();
  double powerToDB(double);
  double powerToAmplitude(double);
  PowerScalingFunction scalePower;
};

}

#endif
