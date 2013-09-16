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
  GlWindow(_argc, _argv, 800, 600),
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
  useAudioInputFile = false;
  echoAudio = false;
  showFPS = false;
  pretrainSecs = 0;
  selectedSceneNum = -1;
  
  gridMapEnabled = true;
  disjointGridMapEnabled = true;
  circleMapEnabled = true;
  beatTrackerEnabled = true;
  eventDetectorEnabled = true;

  int argnr = 1;
  char **argptr = argv + 1;
  char *arg;
  while(argnr < argc) {
    arg = *argptr;
    if(arg[0] == '-') {
      char *argflag = arg + 1;
      if(strcmp(argflag, "h") == 0) {
	usage();
      }
      else if(strcmp(argflag, "f") == 0) {
        useAudioInputFile = true;
        echoAudio = true;
        argnr++; argptr++;
        audioInputFilename = *argptr;
      }
      else if(strcmp(argflag, "b") == 0) {
        argnr++; argptr++;
        audioParameters.bufferSize = atoi(*argptr);
      }
      else if(strcmp(argflag, "d") == 0) {
        argnr++; argptr++;
        audioDeviceName = *argptr;
      }
      else if(strcmp(argflag, "echo") == 0) {
        echoAudio = true;
      }
      else if(strcmp(argflag, "showfps") == 0) {
        showFPS = true;
      }
      else if(strcmp(argflag, "adapt") == 0) {
	argnr++;
	argptr++;
	if(strcmp(*argptr, "time") == 0) {
	  gridMapParameters.adaptationStrategy =
	  disjointGridMapParameters.adaptationStrategy =
	    circleMapParameters.adaptationStrategy =
	    SpectrumMapParameters::TimeBased;
	}
	else if(strcmp(*argptr, "error") == 0) {
	  gridMapParameters.adaptationStrategy =
	  disjointGridMapParameters.adaptationStrategy =
	    circleMapParameters.adaptationStrategy =
	    SpectrumMapParameters::ErrorDriven;
	}
	else {
	  printf("Unknown adaptation strategy %s\n", *argptr);
	  usage();
	}
      }
      else if(strcmp(argflag, "pretrain") == 0) {
	argnr++;
	argptr++;
	pretrainSecs = atof(*argptr);
      }
      else if(strcmp(argflag, "scene") == 0) {
	argnr++;
	argptr++;
	selectedSceneNum = atoi(*argptr);
	selectScene();
      }
      else {
        printf("Unknown option %s\n\n", argflag);
        usage();
      }
    }
    else {
      printf("Unknown parameter %s\n\n", arg);
      usage();
    }
    argptr++;
    argnr++;
  }
}

void Demo::selectScene() {
  gridMapEnabled = false;
  disjointGridMapEnabled = false;
  circleMapEnabled = false;
  beatTrackerEnabled = false;
  eventDetectorEnabled = false;

  switch(selectedSceneNum) {
  case Scene_Mixed:
    printf("cannot select mixed scene\n");
    exit(0);

  case Scene_Dancers:
    circleMapEnabled = true;
    beatTrackerEnabled = true;
    break;

  case Scene_EnlargedCircleMap:
    circleMapEnabled = true;
    break;

  case Scene_EnlargedGridMap:
    gridMapEnabled = true;
    break;

  case Scene_GridMapTrajectory:
    gridMapEnabled = true;
    break;

  case Scene_Isolines:
    gridMapEnabled = true;
    break;

  default:
    printf("unknown scene number %d\n", selectedSceneNum);
    exit(0);
  }
}

void Demo::usage() {
  printf("Usage: %s [options]\n\n", argv[0]);

  printf("Options:\n\n");

  printf(" -f <WAV file> Use audio file as input\n");
  printf(" -b <N>        Set audio buffer size to N (default: %ld)\n", audioParameters.bufferSize);
  printf(" -d <name>     Use specified audio device\n");
  printf(" -echo         Echo audio input back to output\n");
  printf(" -showfps      Output frame rate to console\n");
  printf(" -pretrain <N> Pre-train for N seconds\n");
  printf(" -scene <N>    Select only visualization number N\n");

  exit(0);
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
    switchScene(1);
    break;
    
  case GLUT_KEY_LEFT:
    switchScene(-1);
    break;
  }
}

void Demo::glKeyboard(unsigned char key, int x, int y) {
  switch(key) {
  case 27: // escape
    exit(0);
  }
}

void Demo::switchScene(int step) {
  if(selectedSceneNum == -1)
    moveToScene((sceneNum + step) % numScenes);
  else
    printf("cannot switch scene as a scene was selected\n");
}

void Demo::moveToScene(int _sceneNum) {
  sceneNum = _sceneNum;
  resizeFrames();
}

void Demo::initializeGraphics() {
  normalizeSpectrum = (spectrumAnalyzer->getPowerScale() == SpectrumAnalyzer::Amplitude);
  waveformFrame = new WaveformFrame(spectrumMapInputBuffer, audioParameters.bufferSize);
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

  if(selectedSceneNum == -1)
    moveToScene(Scene_Mixed);
  else
    moveToScene(selectedSceneNum);
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
  if(gridMapEnabled)
    gridMap->feedAudio(inputBuffer, audioParameters.bufferSize);

  if(disjointGridMapEnabled)
    disjointGridMap->feedAudio(inputBuffer, audioParameters.bufferSize);

  if(circleMapEnabled)
    circleMap->feedAudio(inputBuffer, audioParameters.bufferSize);

  if(beatTrackerEnabled)
    beatTracker->feedFeatureVector(spectrumBinDivider->getBinValues());

  if(eventDetectorEnabled)
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
  int pretrainBuffers = pretrainSecs * audioParameters.sampleRate /
    audioParameters.bufferSize;
  for(int i = 0; i < pretrainBuffers; i++) {
    readAudioBufferFromFile();
    processAudioNonThreadSafe(audioFileBuffer);
  }
}

int main(int argc, char **argv) {
  Demo(argc, argv);
}
