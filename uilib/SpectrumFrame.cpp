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

#include "SpectrumFrame.hpp"
#include <math.h>

SpectrumFrame::SpectrumFrame(const SpectrumAnalyzer *_spectrumAnalyzer, bool _normalize) {
  spectrumAnalyzer = _spectrumAnalyzer;
  normalize = _normalize;
}

void SpectrumFrame::render() {
  const float *spectrum = spectrumAnalyzer->getSpectrum();
  static float z;
  static int spectrumBin;
  glShadeModel(GL_FLAT);
  glLineWidth(1.0f);
  for(int i = 0; i < width; i++) {
    spectrumBin = (int) (spectrumAnalyzer->getSpectrumResolution() * i / width);
    z = spectrum[spectrumBin];
    if(normalize) z = normalizer.normalize(log(1.0f + z));
    glColor3f(z, z, z);
    glBegin(GL_LINES);
    vertex2i(i, height);
    vertex2i(i, height - (int) (z * height));
    glEnd();
  }
}
