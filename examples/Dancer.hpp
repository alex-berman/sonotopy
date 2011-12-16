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
  float length;
};

