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

#include "Sonogram.hpp"
#include <string.h>
#include <stdlib.h>
#include <math.h>

using namespace sonotopy;

Sonogram::Sonogram(int _historyLength, int _spectrumResolution) {
  historyLength = _historyLength;
  spectrumResolution = _spectrumResolution;

  sonogramUnshiftedData = new TwoDimArray<float>(historyLength, spectrumResolution);
  sonogramUnshiftedData->fill(0.0f);
  sonogramData = new TwoDimArray<float>(historyLength, spectrumResolution);

  currentSonogramFrame = 0;
}

Sonogram::~Sonogram() {
  delete sonogramUnshiftedData;
  delete sonogramData;
}

void Sonogram::feedSpectrum(const float *spectrum) {
  // Copy the band values to the circular buffer
  sonogramUnshiftedData->setRow(currentSonogramFrame, spectrum);

  // Move pointer forward (or back to beginning if at end)
  currentSonogramFrame = (currentSonogramFrame + 1) % historyLength;

  // Create output buffer by rearranging circular buffer
  const float *sourceSpectrum;
  int sourceRow;
  for(int targetRow = 0; targetRow < historyLength; targetRow++) {
    sourceRow = (currentSonogramFrame + targetRow) % historyLength;
    sourceSpectrum = sonogramUnshiftedData->getRow(sourceRow);
    sonogramData->setRow(targetRow, sourceSpectrum);
  }
}
