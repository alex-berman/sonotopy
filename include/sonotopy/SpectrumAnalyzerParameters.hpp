// Copyright (C) 2013 Alexander Berman
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

#ifndef _SpectrumAnalyzerParameters_hpp_
#define _SpectrumAnalyzerParameters_hpp_

namespace sonotopy {

  typedef enum {
    dB,
    Amplitude
  } PowerScale;

  typedef enum {
    NoWindowFunction,
    BlackmanHarris
  } WindowFunction;


  class SpectrumAnalyzerParameters {
  public:
    SpectrumAnalyzerParameters() {
      powerScale = Amplitude;
      windowFunction = BlackmanHarris;
      windowSize = 16384;
      windowOverlap = (float) 15/16;
    }

    PowerScale powerScale;
    WindowFunction windowFunction;
    int windowSize;
    float windowOverlap;
  };

}

#endif
