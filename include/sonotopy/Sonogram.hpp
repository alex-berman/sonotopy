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

#ifndef _Sonogram_hpp_
#define _Sonogram_hpp_

#include "TwoDimArray.hpp"

namespace sonotopy {

class Sonogram {
public:
  typedef TwoDimArray<float> SonogramData;

  Sonogram(int historyLength, int spectrumResolution);
  ~Sonogram();
  void feedSpectrum(const float *);
  const SonogramData* getSonogramData() const { return sonogramData; }
  int getSpectrumResolution() const { return spectrumResolution; }
  int getHistoryLength() const { return historyLength; }
  int size() const { return spectrumResolution * historyLength; }

private:
  int windowSize;
  int windowsPerFrame;
  int FFTsize;
  int historyLength;
  int spectrumResolution;
  SonogramData *sonogramUnshiftedData;
  SonogramData *sonogramData;
  int currentSonogramFrame;
};

}

#endif
