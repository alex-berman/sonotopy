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

#include "Frame.hpp"
#include <sonotopy/SpectrumBinDivider.hpp>
#include <sonotopy/Normalizer.hpp>

using namespace sonotopy;

class SpectrumBinsFrame : public Frame {
public:
  SpectrumBinsFrame(const SpectrumBinDivider *, bool normalize);
  void render();
private:
  const SpectrumBinDivider *spectrumBinDivider;
  bool normalize;
  Normalizer normalizer;
  unsigned int numBins;
};

