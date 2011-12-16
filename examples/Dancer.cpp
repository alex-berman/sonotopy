#include "Dancer.hpp"
#include <math.h>

using namespace sonotopy;
using namespace std;

const float Dancer::TRACE_LIFETIME = 0.4f;

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
  speedFactor = 0.3 + 0.3 * (float) rand() / RAND_MAX;
  length = 0.03;
  angle = 0;
  angleOffset = 2 * M_PI * (float) rand() / RAND_MAX;
  speedOffset = -(float) rand() / RAND_MAX * 0.2;
  trace.clear();
  currentPos.x = (float) rand() / RAND_MAX;
  currentPos.y = (float) rand() / RAND_MAX;
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
