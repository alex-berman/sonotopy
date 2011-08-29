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

#ifndef EVENTDETECTOR_HPP
#define EVENTDETECTOR_HPP

#include "AudioParameters.hpp"
#include "Smoother.hpp"

namespace sonotopy {

class EventDetector
{
public:
  typedef enum {
    STATE_WAITING_FOR_START,
    STATE_NON_SILENCE,
    STATE_SILENCE
  } State;

  EventDetector(const AudioParameters &);
  void feedAudio(const float *audio, unsigned long numFrames);
  void setDecibelReference(double dB_reference);
  float getDbThreshold();
  void setDbThreshold(float);
  float getTrailingSilenceMs();
  void setTrailingSilenceMs(float);
  float getCurrentDb();
  virtual void onStartOfEvent() {}
  virtual void onEndOfEvent() {}

private:
  AudioParameters audioParameters;
  float amplitudeIntegrationTimeMs;
  float dbThreshold;
  float trailingSilenceMs;
  float minEventDurationMs;
  double dB_defaultReference;
  double dB_reference;
  double log10_min, log10_scalefactor;
  Smoother amplitudeSmoother;
  float db;
  State state;
  float bufferDurationMs;
  float eventDurationMs;
  float silenceDurationMs;

  void updateState();
  double amplitudeToDB(double);
};

}

#endif // EVENTDETECTOR_HPP

