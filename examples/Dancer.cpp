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

#include "Dancer.hpp"
#include <sonotopy/Random.hpp>
#include <math.h>

using namespace sonotopy;
using namespace std;

const float Dancer::TRACE_LIFETIME = 0.4;
const float Dancer::SPEED_FACTOR_MIN = 0.3;
const float Dancer::SPEED_FACTOR_MAX = 0.6;
const float Dancer::SPEED_OFFSET_MIN = -0.2;
const float Dancer::SPEED_OFFSET_MAX = 0;

Dancer::Dancer(CircleMap *_circleMap,
	       BeatTracker *_beatTracker,
	       GlWindow *_window) {
  circleMap = _circleMap;
  beatTracker = _beatTracker;
  window = _window;
  reset();
}

void Dancer::reset() {
  speed = 0;
  speedFactor = randomInRange(SPEED_FACTOR_MIN, SPEED_FACTOR_MAX);
  angle = 0;
  angleOffset = randomInRange(0, 2 * M_PI);
  speedOffset = randomInRange(SPEED_OFFSET_MIN, SPEED_OFFSET_MAX);
  trace.clear();
  currentPos.x = randomInRange(0, 1);
  currentPos.y = randomInRange(0, 1);
  currentTime = 0;
}

void Dancer::update(float timeIncrement) {
  angle = circleMap->getAngle() + angleOffset;
  speed = (beatTracker->getIntensity() + speedOffset) * speedFactor;

  float aspectRatio = (float) window->getHeight() / window->getWidth();

  float distance = speed * timeIncrement;
  currentPos.x += cos(angle) * distance;
  currentPos.y += sin(angle) * distance / aspectRatio;
  currentTime += timeIncrement;
}

void Dancer::render() {
  updateTrace();
  renderTrace();
  if(traceOutOfBounds())
    reset();
}

void Dancer::updateTrace() {
  addCurrentPositionToTrace();
  removeOldTailFromTrace();
}

void Dancer::addCurrentPositionToTrace() {
  Point p;
  p.x = currentPos.x * window->getWidth();
  p.y = currentPos.y * window->getHeight();
  p.startTime = currentTime;
  trace.push_back(p);
}

void Dancer::removeOldTailFromTrace() {
  for(int i = trace.size() - 1; i >= 0; i--) {
    if(currentTime - trace[i].startTime > TRACE_LIFETIME) {
      trace.erase(trace.begin(), trace.begin() + i + 1);
      break;
    }
  }
}

void Dancer::renderTrace() {
  float c;
  glShadeModel(GL_SMOOTH);
  glLineWidth(2.0f);
  glBegin(GL_LINE_STRIP);
  vector<Point>::iterator point = trace.begin();
  glColor3f(0, 0, 0);
  glVertex2f(point->x, point->y);
  point++;
  int traceSize = trace.size();
  int n = 1;
  for(;point != trace.end(); point++) {
    c = (float) (n + 1) / traceSize;
    glColor3f(c, c, c);
    glVertex2f(point->x, point->y);
    n++;
  }
  glEnd();
}

bool Dancer::traceOutOfBounds() {
  return outOfBounds(*(trace.begin())) && outOfBounds(*(trace.end()));
}

bool Dancer::outOfBounds(const Point &p) {
  if(p.x < 0) return true;
  if(p.y < 0) return true;
  if(p.x > window->getWidth()) return true;
  if(p.y > window->getHeight()) return true;
  return false;
}
