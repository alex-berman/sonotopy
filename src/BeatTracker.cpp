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

#include "BeatTracker.hpp"
#include <string.h>
#include <math.h>

using namespace sonotopy;

const float BeatTracker::DEFAULT_ADAPTATION_TIME_MS = 3000.0f;
const float BeatTracker::DEFAULT_RESPONSE_TIME_MS = 80.0f;

BeatTracker::BeatTracker(unsigned int _numFeatures, unsigned int _windowSize, unsigned int _sampleRate) {
  numFeatures = _numFeatures;
  windowSize = _windowSize;
  sampleRate = _sampleRate;
  for(unsigned int i = 0; i < numFeatures; i++)
    previousFeatureVector.push_back(0);
  setAdaptationTimeMs(DEFAULT_ADAPTATION_TIME_MS);
  setResponseTimeMs(DEFAULT_RESPONSE_TIME_MS);
}

BeatTracker::~BeatTracker() {
}

void BeatTracker::setAdaptationTimeMs(float adaptationTimeMs) {
  float adaptationFactor = 1000 * windowSize / sampleRate / adaptationTimeMs;
  if(adaptationFactor >= 1)
    adaptationFactor = 1;
  normalizer.setAdaptationFactor(adaptationFactor);
}

void BeatTracker::setResponseTimeMs(float responseTimeMs) {
  float responseFactor = 1000 * windowSize / sampleRate / responseTimeMs;
  if(responseFactor >= 1)
    responseFactor = 1;
  smoother.setResponseFactor(responseFactor);
}

void BeatTracker::feedFeatureVector(const FeatureVector &latestFeatureVector) {
  float change = compareFeatures(previousFeatureVector, latestFeatureVector);
  previousFeatureVector = latestFeatureVector;
  intensity = smoother.smooth(normalizer.normalize(change));
}

void BeatTracker::feedFeatureVector(const float *features) {
  FeatureVector featureVector;
  const float *featuresPtr = features;
  for(unsigned int i = 0; i < numFeatures; i++)
    featureVector.push_back(*featuresPtr++);
  feedFeatureVector(featureVector);
}

float BeatTracker::getIntensity() const {
  return intensity;
}

float BeatTracker::compareFeatures(const FeatureVector &vector1, const FeatureVector &vector2) {
  float difference = 0.0f;
  FeatureVector::const_iterator p1 = vector1.begin();
  FeatureVector::const_iterator p2 = vector2.begin();
  for(unsigned int i = 0; i < numFeatures; i++) {
    difference += fabsf(*p1 - *p2);
    p1++;
    p2++;
  }
  return difference;
}
