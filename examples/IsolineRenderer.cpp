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

#include "IsolineRenderer.hpp"
#include <math.h>

using namespace std;

IsolineRenderer::IsolineRenderer(IsolineExtractor *_isolineExtractor) {
  isolineExtractor = _isolineExtractor;
  isolinesThresholdValueAuto = 0.5;
}

void IsolineRenderer::getDrawableIsocurveSet(DrawableIsocurveSet &cs) {
  static float score, bestScore, bestThreshold;
  bestScore = -1.0f;
  for(float thr = 0.01f; thr < 0.99f; thr += 0.1f) {
    isolineExtractor->setThreshold(thr);
    isolineExtractor->process();
    score = isolinesGetScore();
    if(score > bestScore) {
      bestScore = score;
      bestThreshold = thr;
    }
  }
  isolinesThresholdValueAuto += (bestThreshold - isolinesThresholdValueAuto) * 0.1f;
  isolineExtractor->setThreshold(isolinesThresholdValueAuto);

  isolineExtractor->process();
  isolineExtractor->interpolate();
  isolineExtractor->smooth(0.5);
  cs.curves = *(isolineExtractor->getCurves());
}

float IsolineRenderer::isolinesGetScore() {
  static IsolineExtractor::CurveSet *curves;
  static float score, s;
  static int n;
  score = 0;
  curves = isolineExtractor->getCurves();
  static IsolineExtractor::CurveSet::iterator ip;
  for(ip = curves->begin(); ip != curves->end(); ip++) {
    n = (int) ip->pixels.size();
    if(ip->enclosed && n > 2) s = (float) n * 100;
    //else s = (float) n; // maximize no. of nodes
    else s = -fabsf((float)n - 10); // strive towards specific no. of nodes
    score += s;
  }
  return score;
}
