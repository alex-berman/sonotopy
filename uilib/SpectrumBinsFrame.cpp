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

#include "SpectrumBinsFrame.hpp"

SpectrumBinsFrame::SpectrumBinsFrame(const SpectrumBinDivider *_spectrumBinDivider, bool _normalize) {
  spectrumBinDivider = _spectrumBinDivider;
  normalize = _normalize;
  numBins = spectrumBinDivider->getNumBins();
}

void SpectrumBinsFrame::render() {
  const float *binValuePtr = spectrumBinDivider->getBinValues();
  static float w;
  static int x1, x2, y1, y2;
  glShadeModel(GL_FLAT);
  y1 = height;
  for(unsigned int i = 0; i < numBins; i++) {
    w = *binValuePtr;
    if(normalize) w = normalizer.normalize(w);
    glColor3f(w, w, w);
    glBegin(GL_POLYGON);
    x1 = (int) (width * i / numBins);
    x2 = (int) (width * (i+1) / numBins);
    y2 = height - (int) (height * w);
    vertex2i(x1, y1);
    vertex2i(x1, y2);
    vertex2i(x2, y2);
    vertex2i(x2, y1);
    vertex2i(x1, y1);
    glEnd();
    binValuePtr++;
  }
}

