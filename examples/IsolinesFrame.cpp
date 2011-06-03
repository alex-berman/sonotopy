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

#include "IsolinesFrame.hpp"
#include <math.h>

using namespace std;

IsolinesFrame::IsolinesFrame(GridMap *_gridMap) {
  gridMap = _gridMap;
  gridMapWidth = gridMap->getParameters().gridWidth;
  gridMapHeight = gridMap->getParameters().gridHeight;
  activationPatternAsTwoDimArray = new TwoDimArray<float>(gridMapWidth, gridMapHeight);
  isolineExtractor = new IsolineExtractor(gridMapWidth, gridMapHeight);
  isolineRenderer = new IsolineRenderer(isolineExtractor);
  lineWidthFactor = 0.1f;
  isocurvesHistoryLength = 7;
  isocurvesHistoryCurrentLength = 0;
}

void IsolinesFrame::render() {
  static IsolineRenderer::DrawableIsocurveSet drawableIsocurveSet;
  activationPatternToTwoDimArray();
  isolineExtractor->setMap(*activationPatternAsTwoDimArray);
  isolineRenderer->getDrawableIsocurveSet(drawableIsocurveSet);
  addDrawableIsocurveSetToHistory(drawableIsocurveSet);
  renderDrawableIsocurveSetHistory();
}

void IsolinesFrame::activationPatternToTwoDimArray() {
  activationPattern = gridMap->getActivationPattern();
  TwoDimArray<float>::Iterator twoDimArrayIterator = activationPatternAsTwoDimArray->begin();
  for(vector<float>::const_iterator vectorIterator = activationPattern->begin();
    vectorIterator != activationPattern->end();
    vectorIterator++)
  {
    *(twoDimArrayIterator->value) = *vectorIterator;
    twoDimArrayIterator++;
  }
}

void IsolinesFrame::addDrawableIsocurveSetToHistory(const IsolineRenderer::DrawableIsocurveSet &drawableIsocurveSet) {
  isocurvesHistory.push_back(drawableIsocurveSet);
  if(isocurvesHistoryCurrentLength == isocurvesHistoryLength)
    isocurvesHistory.erase(isocurvesHistory.begin());
  else
    isocurvesHistoryCurrentLength++;
}

void IsolinesFrame::renderDrawableIsocurveSetHistory() {
  static float is, ic;
  static float ilMin, ilMax;
  static float cx, cy;
  static int px, py;
  ilMin = width * 0.003f;
  ilMax = height * 0.010f * (lineWidthFactor / 0.1f);
  static vector<IsolineRenderer::DrawableIsocurveSet>::iterator ip;
  ip = isocurvesHistory.begin();
  for(int i = 0; i < isocurvesHistoryCurrentLength; i++) {
    is = (float) i / (isocurvesHistoryCurrentLength - 1);
    ic = powf(is, 2.0f);
    glColor3f(ic, ic, ic);
    glLineWidth (ilMin + ilMax * (1.0f-is));
    for(IsolineExtractor::CurveSet::iterator curve = ip->curves.begin(); curve != ip->curves.end(); curve++) {
      glBegin(GL_LINE_STRIP);
      for(vector<IsolineExtractor::Point>::iterator v = curve->points.begin(); v != curve->points.end(); v++) {
        cx = v->x;
        cy = v->y;
        px = (int) (width * cx / (gridMapWidth-1));
        py = (int) (height * cy / (gridMapHeight-1));
        vertex2i(px, py);
      }
      glEnd();
    }
    ip++;
  }
}

