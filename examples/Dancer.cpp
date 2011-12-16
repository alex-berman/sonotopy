#include "Dancer.hpp"
#include <math.h>

using namespace sonotopy;
using namespace std;

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
}

void Dancer::update(float timeIncrement) {
  angle = circleMap->getAngle() + angleOffset;
  speed = (beatTracker->getIntensity() + speedOffset) * speedFactor;

  float aspectRatio = (float) window->getHeight() / window->getWidth();

  float distance = speed * timeIncrement;
  currentPos.x += cos(angle) * distance;
  currentPos.y += sin(angle) * distance / aspectRatio;
}

void Dancer::render() {
  updateTrace();
  renderTrace();
  if(traceOutOfBounds())
    reset();
}

void Dancer::updateTrace() {
  Point p;
  p.x = currentPos.x * window->getWidth();
  p.y = currentPos.y * window->getHeight();
  trace.push_back(p);
  if(trace.size() > 10)
    trace.erase(trace.begin());
}

void Dancer::renderTrace() {
  float c;
  glShadeModel(GL_SMOOTH);
  glLineWidth(2.0f);
  glBegin(GL_LINE_STRIP);
  vector<Point>::iterator pos = trace.begin();
  glColor3f(0, 0, 0);
  glVertex2f(pos->x, pos->y);
  pos++;
  int traceSize = trace.size();
  int n = 1;
  for(;pos != trace.end(); pos++) {
    c = (float) (n + 1) / traceSize;
    glColor3f(c, c, c);
    glVertex2f(pos->x, pos->y);
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
