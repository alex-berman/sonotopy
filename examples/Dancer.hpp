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

#include <sonotopy/sonotopy.hpp>
#include <sonotopy/uilib/uilib.hpp>
#include <vector>

class Dancer {
public:
  Dancer(sonotopy::CircleMap *,
	 sonotopy::BeatTracker *,
	 GlWindow *);
  void update(float timeIncrement);
  void render();
  void reset();

private:
  const static float TRACE_LIFETIME;
  const static float SPEED_FACTOR_MIN;
  const static float SPEED_FACTOR_MAX;
  const static float SPEED_OFFSET_MIN;
  const static float SPEED_OFFSET_MAX;

  typedef struct {
    float x;
    float y;
    float startTime;
  } Point;

  void updateTrace();
  void renderTrace();
  bool outOfBounds(const Point &);
  bool traceOutOfBounds();
  void addCurrentPositionToTrace();
  void removeOldTailFromTrace();

  CircleMap *circleMap;
  BeatTracker *beatTracker;
  GlWindow *window;
  Point currentPos;
  float currentTime;
  std::vector<Point> trace;
  float angleOffset;
  float angle;
  float speed;
  float speedOffset;
  float speedFactor;
};
