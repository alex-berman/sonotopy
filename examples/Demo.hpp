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
#include <sonotopy/uilib/cmdline.hpp>
#include "IsolinesFrame.hpp"
#include "Dancer.hpp"
#include <vector>
#include <pthread.h>

class Demo : public GlWindow, public AudioIO {
public:
  Demo(int _argc, char **_argv);
  ~Demo();
  void processAudio(float *);
  void display();
  void glSpecial(int key, int x, int y);
  void glKeyboard(unsigned char key, int x, int y);
  void resizedWindow();

private:
  enum {
    Scene_Mixed = 0,
    Scene_Dancers,
    Scene_EnlargedCircleMap,
    Scene_EnlargedGridMap,
    Scene_GridMapTrajectory,
    Scene_Isolines,
    numScenes
  };

  class EventDetectionPrinter : public EventDetector {
  public:
    EventDetectionPrinter(const AudioParameters &audioParameters) :
      EventDetector(audioParameters) {}
    void onStartOfEvent() { printf("start of event\n"); }
    void onEndOfEvent()   { printf("end of event\n"); }
  };

  void processCommandLineArguments();
  void usage();
  void initializeAudioProcessing();
  void processAudioNonThreadSafe(float *);
  void createDisjointGridMap();
  void initializeGraphics();
  void mainLoop();
  void resizeFrames();
  void moveToScene(int _sceneNum);
  void updateDancers();
  void renderDancers();
  void pretrain();

  cmdline::parser parser;
  Stopwatch stopwatch;
  int SPACING;
  float SINGLE_FRAME_RELATIVE_SIZE;
  int argc;
  char **argv;
  bool showFPS;
  bool normalizeSpectrum;
  GridMapParameters gridMapParameters;
  GridMapParameters disjointGridMapParameters;
  GridMap *gridMap;
  DisjointGridMap *disjointGridMap;
  const SpectrumAnalyzer *spectrumAnalyzer;
  const SpectrumBinDivider *spectrumBinDivider;
  CircleMapParameters circleMapParameters;
  CircleMap *circleMap;
  BeatTracker *beatTracker;
  EventDetectionPrinter *eventDetector;
  int sceneNum;
  WaveformFrame *waveformFrame;
  SpectrumFrame *spectrumFrame;
  SpectrumBinsFrame *spectrumBinsFrame;
  GridMapFrame *gridMapFrame;
  GridMapFrame *disjointGridMapFrame;
  SmoothGridMapFrame *enlargedGridMapFrame;
  GridMapTrajectoryFrame *gridMapTrajectoryFrame;
  SmoothCircleMapFrame *enlargedCircleMapFrame;
  CircleMapFrame *circleMapFrame;
  BeatTrackerFrame *beatTrackerFrame;
  IsolinesFrame *isolinesFrame;
  std::vector<Dancer> dancers;
  unsigned long displayStartTime;
  unsigned long frameCount;
  float timeOfPreviousDisplay;
  float timeIncrement;
  pthread_mutex_t mutex;
};
