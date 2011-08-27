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

#include "EventDetector.hpp"
#include <math.h>

using namespace sonotopy;

EventDetector::EventDetector(const AudioParameters &_audioParameters) {
  audioParameters = _audioParameters;
  dB_defaultReference = 0.00001;
  amplitudeIntegrationTimeMs = 100;
  dbThreshold = 0.4;
  minEventDurationMs = 0;
  trailingSilenceMs = 2000;

  bufferDurationMs = (float) 1000 * audioParameters.bufferSize
    / audioParameters.sampleRate;
  amplitudeSmoother.setResponseFactor((float) 1000 / audioParameters.sampleRate
				      / amplitudeIntegrationTimeMs);
  setDecibelReference(dB_defaultReference);
  state = STATE_WAITING_FOR_START;
  db = 0;
}

void EventDetector::feedAudio(const float *audio, unsigned long numFrames) {  
  const float *audioP = audio;
  for(unsigned long i = 0; i < numFrames; i++)
    amplitudeSmoother.smooth(fabsf(*audioP++));

  db = amplitudeToDB(amplitudeSmoother.getValue());
  updateState();
}

float EventDetector::getDbThreshold() {
  return dbThreshold;
}

void EventDetector::setDbThreshold(float _dbThreshold) {
  dbThreshold = _dbThreshold;
}

float EventDetector::getTrailingSilenceMs() {
  return trailingSilenceMs;
}

void EventDetector::setTrailingSilenceMs(float _trailingSilenceMs) {
  trailingSilenceMs = _trailingSilenceMs;
}

float EventDetector::getCurrentDb() {
  return db;
}

void EventDetector::updateState() {
  switch(state) {
  case STATE_WAITING_FOR_START:
    if(db > dbThreshold) {
      eventDurationMs = 0;
      state = STATE_NON_SILENCE;
      onStartOfEvent();
    }
    break;

  case STATE_NON_SILENCE:
    eventDurationMs += bufferDurationMs;
    if(db < dbThreshold) {
      silenceDurationMs = 0;
      state = STATE_SILENCE;
    }
    break;

  case STATE_SILENCE:
    eventDurationMs += bufferDurationMs;
    if(db < dbThreshold) {
      silenceDurationMs += bufferDurationMs;
      if(silenceDurationMs > trailingSilenceMs) {
	state = STATE_WAITING_FOR_START;
	eventDurationMs -= trailingSilenceMs;
	if(eventDurationMs > minEventDurationMs)
	  onEndOfEvent();
      }
    }
    else
      state = STATE_NON_SILENCE;
    break;
  }
}

void EventDetector::setDecibelReference(double _dB_reference) {
  dB_reference = _dB_reference;
  log10_min = log10(dB_reference);
  double log10_max = log10(1.0f);
  log10_scalefactor = log10_max - log10_min;
}

double EventDetector::amplitudeToDB(double x) {
  if(x < dB_reference) x = dB_reference;
  return (log10(x) - log10_min) / log10_scalefactor;
}
