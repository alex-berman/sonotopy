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
#include "GlWindow.hpp"
#include "AudioIO.hpp"
#include "Frame.hpp"
#include "WaveformFrame.hpp"
#include "SpectrumFrame.hpp"
#include "SpectrumBinsFrame.hpp"
#include "GridMapFrame.hpp"
#include "SmoothGridMapFrame.hpp"
#include "IsolinesFrame.hpp"
#include "GridMapTrajectoryFrame.hpp"
#include "CircleMapFrame.hpp"
#include "SmoothCircleMapFrame.hpp"
#include "BeatTrackerFrame.hpp"
#include <vector>

class Demo : public GlWindow, public AudioIO {
public:
  Demo(int _argc, char **_argv);
  ~Demo();
  void processAudio(float *);
  void glDisplay();
  void glReshape(int width, int height);
  void glSpecial(int key, int x, int y);
  int getWindowWidth() { return windowWidth; }
  int getWindowHeight() { return windowHeight; }

private:
  class ErrorPlotter {
  public:
    ErrorPlotter(const Demo *, const SpectrumMap *);
    ~ErrorPlotter();
    void update();
    void render(Frame *);

  private:
    const SpectrumMap *map;
    Frame *frame;
    float *buffer;
    CircularBuffer<float> *circularBufferMin;
    CircularBuffer<float> *circularBufferMax;
    int bufferSize;
    float maxValue;
    float maxValueInGraph;
    Smoother smootherMin, smootherMax;
  };

  typedef struct {
    float x;
    float y;
  } Point;

  class Dancer {
  public:
    Dancer(Demo *);
    void update(float timeIncrement);
    void render();
    void reset();

  private:
    void updateTrace();
    void renderTrace();
    bool outOfBounds(const Point &);
    bool traceOutOfBounds();

    Demo *parent;
    Point currentPos;
    std::vector<Point> trace;
    float angleOffset;
    float angle;
    float speed;
    float speedOffset;
    float speedFactor;
    float length;
  };

  enum {
    Scene_Mixed = 0,
    Scene_Dancers,
    Scene_EnlargedCircleMap,
    Scene_EnlargedGridMap,
    Scene_GridMapTrajectory,
    Scene_Isolines,
    numScenes
  };

  void processCommandLineArguments();
  void usage();
  void initializeAudioProcessing();
  void initializeGraphics();
  void mainLoop();
  void resizeFrames();
  void moveToScene(int _sceneNum);
  void updateDancers();
  void renderDancers();

  Stopwatch stopwatch;
  int SPACING;
  float SINGLE_FRAME_RELATIVE_SIZE;
  int argc;
  char **argv;
  bool showFPS;
  bool showAdaptationValues;
  bool normalizeSpectrum;
  GridMapParameters gridMapParameters;
  GridMap *gridMap;
  const SpectrumAnalyzer *spectrumAnalyzer;
  const SpectrumBinDivider *spectrumBinDivider;
  CircleMapParameters circleMapParameters;
  CircleMap *circleMap;
  BeatTracker *beatTracker;
  int sceneNum;
  WaveformFrame *waveformFrame;
  SpectrumFrame *spectrumFrame;
  SpectrumBinsFrame *spectrumBinsFrame;
  GridMapFrame *gridMapFrame;
  SmoothGridMapFrame *enlargedGridMapFrame;
  GridMapTrajectoryFrame *gridMapTrajectoryFrame;
  SmoothCircleMapFrame *enlargedCircleMapFrame;
  CircleMapFrame *circleMapFrame;
  BeatTrackerFrame *beatTrackerFrame;
  IsolinesFrame *isolinesFrame;
  unsigned long frameCount;
  unsigned long displayStartTime;
  int windowWidth, windowHeight;
  std::vector<Dancer> dancers;
  float timeOfPreviousDisplay;
  float timeIncrement;
  bool plotError;
  ErrorPlotter *circleMapErrorPlotter;
  ErrorPlotter *gridMapErrorPlotter;
};
