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

#include "GridMapTrajectoryFrame.hpp"

using namespace std;

GridMapTrajectoryFrame::GridMapTrajectoryFrame(GridMap *_gridMap) {
  gridMap = _gridMap;
}

void GridMapTrajectoryFrame::render() {
  updateTrace();
  renderTrace();
}

void GridMapTrajectoryFrame::updateTrace() {
  Point p;
  float wx, wy;
  gridMap->getCursor(wx, wy);
  p.x = wx * width;
  p.y = wy * height;
  trace.push_back(p);
  if(trace.size() > 10)
    trace.erase(trace.begin());
}

void GridMapTrajectoryFrame::renderTrace() {
  float c;
  glShadeModel(GL_SMOOTH);
  glLineWidth(3.0f);
  glBegin(GL_LINE_STRIP);
  vector<Point>::iterator pos = trace.begin();
  glColor3f(0, 0, 0);
  vertex2f(pos->x, pos->y);
  pos++;
  int traceSize = trace.size();
  int n = 1;
  for(;pos != trace.end(); pos++) {
    c = (float) (n + 1) / traceSize;
    glColor3f(c, c, c);
    vertex2f(pos->x, pos->y);
    n++;
  }
  glEnd();
}
