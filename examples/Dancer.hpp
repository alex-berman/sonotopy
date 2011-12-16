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
  typedef struct {
    float x;
    float y;
  } Point;

  void updateTrace();
  void renderTrace();
  bool outOfBounds(const Point &);
  bool traceOutOfBounds();

  CircleMap *circleMap;
  BeatTracker *beatTracker;
  GlWindow *window;
  Point currentPos;
  std::vector<Point> trace;
  float angleOffset;
  float angle;
  float speed;
  float speedOffset;
  float speedFactor;
  float length;
};

