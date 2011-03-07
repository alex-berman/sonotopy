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

#include "IsolineExtractor.hpp"
#include "IsolineRenderer.hpp"
#include "Frame.hpp"
#include <sonotopy/GridMap.hpp>
#include <sonotopy/TwoDimArray.hpp>
#include <vector>

using namespace sonotopy;

class IsolinesFrame : public Frame {
public:
  IsolinesFrame(GridMap *gridMap);
  void render();

private:
  void addDrawableIsocurveSetToHistory(const IsolineRenderer::DrawableIsocurveSet &);
  void renderDrawableIsocurveSetHistory();
  void activationPatternToTwoDimArray();

  GridMap *gridMap;
  int gridMapWidth;
  int gridMapHeight;
  float lineWidthFactor;
  const SOM::ActivationPattern *activationPattern;
  TwoDimArray<float> *activationPatternAsTwoDimArray;
  IsolineExtractor *isolineExtractor;
  IsolineRenderer *isolineRenderer;
  int isocurvesHistoryLength;
  std::vector<IsolineRenderer::DrawableIsocurveSet> isocurvesHistory;
  int isocurvesHistoryCurrentLength;
};

