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

#ifndef _BeatTracker_hpp_
#define _BeatTracker_hpp_

#include "Normalizer.hpp"
#include "Smoother.hpp"
#include <vector>

namespace sonotopy {

class BeatTracker {
public:
  typedef std::vector<float> FeatureVector;

  BeatTracker(unsigned int numFeatures, unsigned int windowSize, unsigned int sampleRate);
  ~BeatTracker();
  void feedFeatureVector(const FeatureVector &);
  void feedFeatureVector(const float *);
  float getIntensity() const;
  void setResponseTimeMs(float);
  void setAdaptationTimeMs(float);

private:

  float compareFeatures(const FeatureVector &, const FeatureVector &);

  unsigned int numFeatures;
  unsigned int windowSize;
  unsigned int sampleRate;
  FeatureVector previousFeatureVector;
  float intensity;
  Normalizer normalizer;
  Smoother smoother;
};

}

#endif
