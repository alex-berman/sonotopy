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
#include "Frame.hpp"
#include "IsolineExtractor.hpp"
#include "IsolineRenderer.hpp"
#include <portaudio.h>
#include <sndfile.h>
#include <vector>

class Demo : public GlWindow {
public:
  const static float activationPatternContrast;

  Demo(int _argc, char **_argv);
  ~Demo();
  int audioCallback(float *inputBuffer, float *outputBuffer, unsigned long framesPerBuffer);
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

  class WaveformFrame : public Frame {
  public:
    WaveformFrame(Demo *);
    void render();
  private:
    Demo *parent;
  };

  class SpectrumFrame : public Frame {
  public:
    SpectrumFrame(Demo *);
    void render();
  private:
    float normalizeValue(float);
    Demo *parent;
    Normalizer normalizer;
  };

  class SpectrumBinsFrame : public Frame {
  public:
    SpectrumBinsFrame(Demo *);
    void render();
  private:
    Demo *parent;
    Normalizer normalizer;
  };

  class GridMapFrame : public Frame {
  public:
    GridMapFrame(Demo *);
    ~GridMapFrame();
    void render();
  private:
    void renderActivationPattern();
    void renderCursor();
    Demo *parent;
  };

  class SmoothGridMapFrame : public Frame {
  public:
    SmoothGridMapFrame(Demo *);
    ~SmoothGridMapFrame();
    void render();
  private:
    void setColorFromActivationPattern(int x, int y);
    Demo *parent;
  };

  class GridMapTrajectoryFrame : public Frame {
  public:
    GridMapTrajectoryFrame(Demo *);
    ~GridMapTrajectoryFrame();
    void render();
  private:
    Demo *parent;
    typedef struct {
      float x;
      float y;
    } Point;
    std::vector<Point> trace;
    void updateTrace();
    void renderTrace();
  };

  class CircleMapFrame : public Frame {
  public:
    CircleMapFrame(Demo *);
    ~CircleMapFrame();
    void render();
  private:
    Demo *parent;
    int numNodes;
  };

  class SmoothCircleMapFrame : public Frame {
  public:
    SmoothCircleMapFrame(Demo *);
    ~SmoothCircleMapFrame();
    void render();
  private:
    float getColorAtAngle(float);
    Demo *parent;
    int numNodes;
    float angleIncrement;
  };

  class BeatTrackerFrame : public Frame {
  public:
    BeatTrackerFrame(Demo *);
    void render();

  private:
    Demo *parent;
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

  class IsolinesFrame : public Frame {
  public:
    IsolinesFrame(Demo *);
    void render();

  private:
    void addDrawableIsocurveSetToHistory(const IsolineRenderer::DrawableIsocurveSet &);
    void renderDrawableIsocurveSetHistory();
    void activationPatternToTwoDimArray();

    float lineWidthFactor;
    Demo *parent;
    TwoDimArray<float> *activationPatternAsTwoDimArray;
    IsolineExtractor *isolineExtractor;
    IsolineRenderer *isolineRenderer;
    int isocurvesHistoryLength;
    std::vector<IsolineRenderer::DrawableIsocurveSet> isocurvesHistory;
    int isocurvesHistoryCurrentLength;
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
  void initializeAudio();
  void openAudioInputFile();
  void openAudioStream();
  void initializeAudioProcessing();
  void initializeGraphics();
  void mainLoop();
  void readAudioBufferFromFile();
  void resizeFrames();
  void moveToScene(int _sceneNum);
  void updateDancers();
  void renderDancers();

  Stopwatch stopwatch;
  int SPACING;
  float SINGLE_FRAME_RELATIVE_SIZE;
  int argc;
  char **argv;
  bool useAudioInputFile;
  bool showFPS;
  bool showAdaptationValues;
  char *audioInputFilename;
  AudioParameters audioParameters;
  bool normalizeSpectrum;
  GridMapParameters gridMapParameters;
  GridMap *gridMap;
  const SOM::ActivationPattern *gridMapActivationPattern;
  int gridMapWidth;
  int gridMapHeight;
  const SpectrumAnalyzer *spectrumAnalyzer;
  const SpectrumBinDivider *spectrumBinDivider;
  CircleMapParameters circleMapParameters;
  CircleTopology *circleTopology;
  CircleMap *circleMap;
  const SOM::ActivationPattern *circleMapActivationPattern;
  BeatTracker *beatTracker;
  SNDFILE *audioInputFile;
  float *audioFileBuffer;
  PaStream *paStream;
  const char *audioDeviceName;
  float *spectrumMapInputBuffer;
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
  bool echoAudio;
  float timeOfPreviousDisplay;
  float timeIncrement;
  bool plotError;
  ErrorPlotter *circleMapErrorPlotter;
  ErrorPlotter *gridMapErrorPlotter;
};
