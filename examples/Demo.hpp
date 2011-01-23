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
  Demo(int _argc, char **_argv);
  ~Demo();
  int audioCallback(float *inputBuffer, float *outputBuffer, unsigned long framesPerBuffer);
  void glDisplay();
  void glReshape(int width, int height);
  void glSpecial(int key, int x, int y);
  int getWindowWidth() { return windowWidth; }
  int getWindowHeight() { return windowHeight; }

private:
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

  class SonogramFrame : public Frame {
  public:
    SonogramFrame(Demo *);
    void render();
  private:
    Demo *parent;
    Normalizer normalizer;
  };

  class SonogramGridMapFrame : public Frame {
  public:
    SonogramGridMapFrame(Demo *);
    ~SonogramGridMapFrame();
    void render();
  private:
    Demo *parent;
  };

  class SmoothSonogramGridMapFrame : public Frame {
  public:
    SmoothSonogramGridMapFrame(Demo *);
    ~SmoothSonogramGridMapFrame();
    void render();
  private:
    void setColorFromActivationPattern(int x, int y);
    Demo *parent;
  };

  class SonogramCircleMapFrame : public Frame {
  public:
    SonogramCircleMapFrame(Demo *);
    ~SonogramCircleMapFrame();
    void render();
  private:
    Demo *parent;
    int numNodes;
  };

  class SmoothSonogramCircleMapFrame : public Frame {
  public:
    SmoothSonogramCircleMapFrame(Demo *);
    ~SmoothSonogramCircleMapFrame();
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
    float vaneAngle;
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
    Scene_Vane,
    Scene_EnlargedSonogramCircleMap,
    Scene_EnlargedSonogramGridMap,
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
  void updateVaneScene(float timeIncrement);
  void renderVaneScene();

  Stopwatch stopwatch;
  int SPACING;
  float SINGLE_FRAME_RELATIVE_SIZE;
  int argc;
  char **argv;
  bool useAudioInputFile;
  bool showFPS;
  char *audioInputFilename;
  AudioParameters audioParameters;
  bool normalizeSpectrum;
  GridMapCircuitParameters gridMapCircuitParameters;
  RectGridTopology *gridTopology;
  GridMapCircuit *sonogramGridMapCircuit;
  const SonogramMap::ActivationPattern *gridMapActivationPattern;
  const SonogramMap *sonogramGridMap;
  int sonogramGridMapWidth;
  int sonogramGridMapHeight;
  const SpectrumAnalyzer *spectrumAnalyzer;
  const SpectrumBinDivider *spectrumBinDivider;
  const Sonogram *sonogram;
  CircleMapCircuitParameters circleMapCircuitParameters;
  CircleTopology *circleTopology;
  CircleMapCircuit *sonogramCircleMapCircuit;
  const SonogramMap::ActivationPattern *circleMapActivationPattern;
  const SonogramMap *sonogramCircleMap;
  BeatTracker *beatTracker;
  Vane *vane;
  SNDFILE *audioInputFile;
  float *audioFileBuffer;
  PaStream *paStream;
  int audioDevice;
  float *sonogramMapCircuitInputBuffer;
  int sceneNum;
  WaveformFrame *waveformFrame;
  SpectrumFrame *spectrumFrame;
  SpectrumBinsFrame *spectrumBinsFrame;
  SonogramFrame *sonogramFrame;
  SonogramGridMapFrame *sonogramGridMapFrame;
  SmoothSonogramGridMapFrame *enlargedSonogramGridMapFrame;
  SmoothSonogramCircleMapFrame *enlargedSonogramCircleMapFrame;
  SonogramCircleMapFrame *sonogramCircleMapFrame;
  BeatTrackerFrame *beatTrackerFrame;
  IsolinesFrame *isolinesFrame;
  unsigned long frameCount;
  unsigned long displayStartTime;
  int windowWidth, windowHeight;
  std::vector<Dancer> dancers;
  bool echoAudio;
};
