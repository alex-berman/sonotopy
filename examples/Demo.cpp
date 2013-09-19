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

#include "Demo.hpp"
#include <string.h>
#include <time.h>

using namespace std;
using namespace sonotopy;

Demo::Demo(int _argc, char **_argv) :
  GlWindow(_argc, _argv),
  AudioIO()
{
  argc = _argc;
  argv = _argv;
  
  SPACING = 5;
  SINGLE_FRAME_RELATIVE_SIZE = 0.8;
  frameCount = 0;

  processCommandLineArguments();
  initializeAudioProcessing();
  initializeAudio();
  initializeGraphics();
  pretrain();
  openAudioStream();
  mainLoop();
}

Demo::~Demo() {
}

void Demo::processCommandLineArguments() {
  parser.add<string>("audiofile", 'f', "Audio file for input", false);
  parser.add<int>("buffersize", 'b', "Audio buffer size", false, audioParameters.bufferSize);
  parser.add<string>("audiodevice", 'd', "Audio device", false);
  parser.add("echo", '\0', "Echo audio input back to output");
  parser.add("showfps", '\0', "Output frame rate to console");
  parser.add<float>("pretrain", '\0', "Pre-train for N seconds", false, 0.0);
  parser.add<int>("width", 'w', "Window width", false, 800);
  parser.add<int>("height", 'h', "Window height", false, 600);
  parser.add("export", '\0', "Export video");
  parser.parse_check(argc, argv);

  if(parser.exist("audiofile")) {
    useAudioInputFile = true;
    audioInputFilename = parser.get<string>("audiofile").c_str();
  }
  else {
    useAudioInputFile = false;
  }

  echoAudio = (useAudioInputFile || parser.exist("echo"));

  audioDeviceName = parser.get<string>("audiodevice").c_str();
  showFPS = parser.exist("showfps");

  if(parser.exist("export")) {
    audioEnableVideoExport();
    windowEnableVideoExport();
  }
}

void Demo::initializeAudioProcessing() {
  srand((unsigned) time(NULL));
  pthread_mutex_init(&mutex, NULL);

  gridMap = new GridMap(audioParameters, gridMapParameters);
  spectrumAnalyzer = gridMap->getSpectrumAnalyzer();
  spectrumBinDivider = gridMap->getSpectrumBinDivider();
  circleMap = new CircleMap(audioParameters, circleMapParameters);
  beatTracker = new BeatTracker(spectrumBinDivider->getNumBins(), audioParameters.bufferSize, audioParameters.sampleRate);
  eventDetector = new EventDetectionPrinter(audioParameters);
  createDisjointGridMap();
}

void Demo::createDisjointGridMap() {
  // create disjoint map consisting of two sections: upper-left and bottom-right corner
  int w = disjointGridMapParameters.gridWidth * 0.8;
  int h = disjointGridMapParameters.gridHeight * 0.8;
  vector<DisjointGridTopology::Node> nodes;
  for(int y = 0; y < h; y++) {
    int rw = w * y / h;
    for(int x = 0; x < rw; x++) {
      nodes.push_back(DisjointGridTopology::Node(disjointGridMapParameters.gridWidth-x-1, h-y));
      nodes.push_back(DisjointGridTopology::Node(x, disjointGridMapParameters.gridHeight-(h-y)-1));
    }
  }
  disjointGridMap = new DisjointGridMap(audioParameters, disjointGridMapParameters, nodes);
}

void Demo::glSpecial(int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_RIGHT:
    if(sceneNum == (numScenes - 1))
      moveToScene(0);
    else
      moveToScene(sceneNum + 1);
    break;

  case GLUT_KEY_LEFT:
    if(sceneNum == 0)
      moveToScene(numScenes - 1);
    else
      moveToScene(sceneNum - 1);
    break;
  }
}

void Demo::glKeyboard(unsigned char key, int x, int y) {
  switch(key) {
  case 27: // escape
    exit(0);
  }
}

void Demo::moveToScene(int _sceneNum) {
  sceneNum = _sceneNum;
  resizeFrames();
}

void Demo::initializeGraphics() {
  setWindowSize(parser.get<int>("width"), parser.get<int>("height"));
  GlWindow::initializeGraphics();

  normalizeSpectrum = (spectrumAnalyzer->getPowerScale() == SpectrumAnalyzer::Amplitude);
  waveformFrame = new WaveformFrame(monauralInputBuffer, audioParameters.bufferSize);
  spectrumFrame = new SpectrumFrame(spectrumAnalyzer, normalizeSpectrum);
  spectrumBinsFrame = new SpectrumBinsFrame(spectrumBinDivider, normalizeSpectrum);
  gridMapFrame = new GridMapFrame(gridMap);
  disjointGridMapFrame = new GridMapFrame(disjointGridMap);
  enlargedGridMapFrame = new SmoothGridMapFrame(gridMap);
  gridMapTrajectoryFrame = new GridMapTrajectoryFrame(gridMap);
  beatTrackerFrame = new BeatTrackerFrame(beatTracker);
  isolinesFrame = new IsolinesFrame(gridMap);
  circleMapFrame = new CircleMapFrame(circleMap, beatTracker);
  enlargedCircleMapFrame = new SmoothCircleMapFrame(circleMap);

  for(int i = 0; i < 20; i++)
    dancers.push_back(Dancer(circleMap, beatTracker, this));

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);

  moveToScene(Scene_Mixed);
}

void Demo::resizedWindow() {
  resizeFrames();
}

void Demo::resizeFrames() {
  int singleFrameSize = (int) (SINGLE_FRAME_RELATIVE_SIZE * min(windowWidth, windowHeight));
  int singleFrameOffsetLeft = (windowWidth - singleFrameSize) / 2;
  int singleFrameOffsetTop = (windowHeight - singleFrameSize) / 2;

  int numRows = 4;
  int displayWidth = windowWidth - SPACING*2;
  int displayHeight = windowHeight - SPACING*(numRows+1);
  int rowHeight = (int) (displayHeight / 4);
  int y0 = SPACING;
  int y1 = y0 + rowHeight + SPACING;
  int y2 = y1 + rowHeight + SPACING;
  int y3 = y2 + rowHeight + SPACING;
  int columnWidth = rowHeight;

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->setSize(displayWidth, rowHeight);
      waveformFrame->setPosition(SPACING, y0);

      spectrumFrame->setSize(displayWidth, rowHeight);
      spectrumFrame->setPosition(SPACING, y1);

      spectrumBinsFrame->setSize(displayWidth, rowHeight);
      spectrumBinsFrame->setPosition(SPACING, y2);

      beatTrackerFrame->setSize(columnWidth / 2, rowHeight);
      beatTrackerFrame->setPosition(SPACING, y3);

      circleMapFrame->setSize(columnWidth, rowHeight);
      circleMapFrame->setPosition(SPACING + displayWidth - columnWidth*3 - SPACING*2, y3);

      gridMapFrame->setSize(columnWidth, rowHeight);
      gridMapFrame->setPosition(SPACING + displayWidth - columnWidth*2 - SPACING, y3);

      disjointGridMapFrame->setSize(columnWidth, rowHeight);
      disjointGridMapFrame->setPosition(SPACING + displayWidth - columnWidth, y3);
      break;

    case Scene_EnlargedCircleMap:
      enlargedCircleMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedCircleMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_EnlargedGridMap:
      enlargedGridMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedGridMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_GridMapTrajectory:
      gridMapTrajectoryFrame->setSize(singleFrameSize, singleFrameSize);
      gridMapTrajectoryFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_Isolines:
      isolinesFrame->setSize(singleFrameSize, singleFrameSize);
      isolinesFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;
  }
}

void Demo::mainLoop() {
  printf("press right or left arrow on keyboard to switch between visualizations\n");
  glutMainLoop();
}

void Demo::processAudio(float *inputBuffer) {
  pthread_mutex_lock(&mutex);
  processAudioNonThreadSafe(inputBuffer);
  pthread_mutex_unlock(&mutex);
}

void Demo::processAudioNonThreadSafe(float *inputBuffer) {
  gridMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  disjointGridMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  circleMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  beatTracker->feedFeatureVector(spectrumBinDivider->getBinValues());
  eventDetector->feedAudio(inputBuffer, audioParameters.bufferSize);
}

void Demo::display() {
  if(pthread_mutex_trylock(&mutex) != 0)
    return;

  if(frameCount == 0) {
    stopwatch.start();
    timeIncrement = 0;
  }
  else {
    float timeOfThisDisplay = stopwatch.getElapsedMilliseconds();
    timeIncrement = timeOfThisDisplay - timeOfPreviousDisplay;
    timeOfPreviousDisplay = timeOfThisDisplay;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  if(exportEnabled) {
    readAudioBufferFromFile();
    processAudioNonThreadSafe(monauralInputBuffer);
  }

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->display();
      spectrumFrame->display();
      spectrumBinsFrame->display();
      gridMapFrame->display();
      disjointGridMapFrame->display();
      circleMapFrame->display();
      beatTrackerFrame->display();
      break;

    case Scene_Dancers:
      updateDancers();
      renderDancers();
      break;

    case Scene_EnlargedCircleMap:
      enlargedCircleMapFrame->display();
      break;

    case Scene_EnlargedGridMap:
      enlargedGridMapFrame->display();
      break;

    case Scene_GridMapTrajectory:
      gridMapTrajectoryFrame->display();
      break;

    case Scene_Isolines:
      isolinesFrame->display();
      break;
  }

  pthread_mutex_unlock(&mutex);
  glutSwapBuffers();
  frameCount++;

  if(showFPS) {
    if(frameCount % 100 == 0) {
      float FPS = (float)frameCount / stopwatch.getElapsedMilliseconds() * 1000;
      printf("fps=%.3f\n", FPS);
    }
  }
}

void Demo::updateDancers() {
  float timeIncrementSecs = timeIncrement / 1000;
  for(vector<Dancer>::iterator dancer = dancers.begin(); dancer != dancers.end(); dancer++)
    dancer->update(timeIncrementSecs);
}

void Demo::renderDancers() {
  for(vector<Dancer>::iterator dancer = dancers.begin(); dancer != dancers.end(); dancer++)
    dancer->render();
}

void Demo::pretrain() {
  int pretrainBuffers = parser.get<float>("pretrain") * audioParameters.sampleRate /
    audioParameters.bufferSize;
  if(pretrainBuffers > 0) {
    printf("pre-training...\n");
    for(int i = 0; i < pretrainBuffers; i++) {
      readAudioBufferFromFile();
      processAudioNonThreadSafe(audioFileBuffer);
    }
    rewindAudioInputFile();
    printf("ok\n");
  }
}

int main(int argc, char **argv) {
  Demo(argc, argv);
}
