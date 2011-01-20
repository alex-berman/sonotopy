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

#include "SpectrumAnalyzer.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>

using namespace sonotopy;

SpectrumAnalyzer::SpectrumAnalyzer(
    int _windowSize,
    float _windowOverlap,
    PowerScale _powerScale,
    WindowFunction _windowFunction) {
  windowSize = _windowSize;
  windowOverlap = _windowOverlap;
  powerScale = _powerScale;
  windowFunction = _windowFunction;

  spectrumResolution = windowSize / 2;
  numUnconsumedFrames = 0;
  numNewFramesPerFFT = (unsigned long) (windowSize * (1.0f-windowOverlap));

  dB_defaultReference = 0.00001;

  inputHistoryBuffer = (float *) malloc(sizeof(float) * windowSize);
  inputHistory = new CircularBuffer<float> (windowSize);
  fftIn = (fftw_complex *) malloc(sizeof(fftw_complex) * windowSize);
  fftOut = (fftw_complex *) malloc(sizeof(fftw_complex) * windowSize);
  fftPlan = fftw_plan_dft_1d(windowSize, fftIn, fftOut, FFTW_DHT, FFTW_MEASURE);
  spectrum = (float *) malloc(sizeof(float) * spectrumResolution);

  if(powerScale == dB) {
    setDecibelReference(dB_defaultReference);
    scalePower = &SpectrumAnalyzer::powerToDB;
  }
  else {
    fftOutMax = windowSize;
    scalePower = &SpectrumAnalyzer::powerToAmplitude;
  }

  if(windowFunction == BlackmanHarris)
    createBlackmanHarrisWindowFunctionTable();
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
  free(inputHistoryBuffer);
  delete inputHistory;
  free(fftIn);
  free(fftOut);
  free(spectrum);
  if(windowFunction != NoWindowFunction)
    delete [] windowFunctionTable;
}

void SpectrumAnalyzer::setDecibelReference(double _dB_reference) {
  dB_reference = _dB_reference;
  log10_min = log10(dB_reference);
  double log10_max = log10((float)windowSize*windowSize);
  log10_scalefactor = log10_max - log10_min;
}

void SpectrumAnalyzer::feedAudioFrames(const float *inputBuffer, unsigned long numFrames) {
  appendAudioToHistory(inputBuffer, numFrames);
  processUnconsumedFrames();
}

void SpectrumAnalyzer::processUnconsumedFrames() {
  while(numUnconsumedFrames >= numNewFramesPerFFT) {
    inputHistory->moveReadHead(numNewFramesPerFFT);
    numUnconsumedFrames -= numNewFramesPerFFT;
    performFFT();
  }
}

void SpectrumAnalyzer::appendAudioToHistory(const float *inputBuffer, unsigned long numFrames) {
  inputHistory->write(numFrames, inputBuffer);
  numUnconsumedFrames += numFrames;
}

void SpectrumAnalyzer::performFFT() {
  inputHistoryToFftIn();
  fftw_execute(fftPlan);
  fftOutToSpectrum();
}

void SpectrumAnalyzer::inputHistoryToFftIn() {
  inputHistory->read(windowSize, inputHistoryBuffer);
  float *inputPtr = inputHistoryBuffer;
  fftw_complex *fftInPtr = fftIn;
  for(int i = 0; i < windowSize; i++) {
    (*fftInPtr)[0] = *inputPtr++;
    (*fftInPtr)[1] = 0;
    fftInPtr++;
  }
  if(windowFunction != NoWindowFunction) {
    double *tableP = windowFunctionTable;
    fftInPtr = fftIn;
    for(int i = 0; i < windowSize; i++) {
      (*fftInPtr)[0] *= (*tableP);
      fftInPtr++;
      tableP++;
    }
  }
}

void SpectrumAnalyzer::fftOutToSpectrum() {
  fftw_complex *fftOutPtr = fftOut;
  float *spectrumPtr = spectrum;
  double r, c;
  for(unsigned long i = 0; i < spectrumResolution; i++) {
    r = (*fftOutPtr)[0];
    c = (*fftOutPtr)[1];
    *spectrumPtr++ = (float) (this->*scalePower)(r*r+c*c);
    fftOutPtr++;
  }
}

double SpectrumAnalyzer::powerToAmplitude(double x) {
  return sqrt(x) / fftOutMax;
}

double SpectrumAnalyzer::powerToDB(double x) {
  if(x < dB_reference) x = dB_reference;
  return (log10(x) - log10_min) / log10_scalefactor;
}

void SpectrumAnalyzer::createBlackmanHarrisWindowFunctionTable() {
  // GNUPLOT: plot [0:(N-1)] N=1024, 0.35875-0.48829*cos(2*pi*x/(N-1))+0.14128*cos(4*pi*x/(N-1))-0.01168*cos(6*pi*x/(N-1));
  windowFunctionTable = new double [windowSize];
  double *tableP = windowFunctionTable;
  for(int x = 0; x < windowSize; x++) {
    *tableP =
      0.35875
      - 0.48829 * cos(2*M_PI*x/(windowSize-1))
      + 0.14128 * cos(4*M_PI*x/(windowSize-1))
      - 0.01168 * cos(6*M_PI*x/(windowSize-1));
    tableP++;
  }
}

