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
#include <math.h>
#include <time.h>

using namespace std;
using namespace sonotopy;

Demo *demo;

Demo::Demo(int _argc, char **_argv) : GlWindow(_argc, _argv, 800, 600) {
  demo = this;
  argc = _argc;
  argv = _argv;
  audioFileBuffer = NULL;
  spectrumMapInputBuffer = NULL;
  audioDeviceName = NULL;

  SPACING = 5;
  SINGLE_FRAME_RELATIVE_SIZE = 0.8;

  processCommandLineArguments();
  if(useAudioInputFile) openAudioInputFile();
  initializeAudioProcessing();
  initializeAudio();
  initializeGraphics();
  openAudioStream();
  mainLoop();
}

Demo::~Demo() {
  if(useAudioInputFile) sf_close(audioInputFile);
  if(audioFileBuffer) delete audioFileBuffer;
  if(spectrumMapInputBuffer) delete spectrumMapInputBuffer;
}

void Demo::processCommandLineArguments() {
  useAudioInputFile = false;
  echoAudio = false;
  showFPS = false;
  showAdaptationValues = false;
  plotError = false;
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
      else if(strcmp(argflag, "error") == 0) {
        plotError = true;
      }
      else if(strcmp(argflag, "adapt") == 0) {
	argnr++;
	argptr++;
	if(strcmp(*argptr, "time") == 0) {
	  gridMapParameters.adaptationStrategy = circleMapParameters.adaptationStrategy =
	    SpectrumMapParameters::TimeBased;
	}
	else if(strcmp(*argptr, "error") == 0) {
	  gridMapParameters.adaptationStrategy = circleMapParameters.adaptationStrategy =
	    SpectrumMapParameters::ErrorDriven;
	}
	else {
	  printf("Unknown adaptation strategy %s\n", *argptr);
	  usage();
	}
      }
      else if(strcmp(argflag, "showadapt") == 0) {
	showAdaptationValues = true;
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

void Demo::usage() {
  printf("Usage: %s [options]\n\n", argv[0]);

  printf("Options:\n\n");

  printf(" -f <WAV file> Use audio file as input\n");
  printf(" -b <N>        Set audio buffer size to N (default: %ld)\n", audioParameters.bufferSize);
  printf(" -d <name>     Use specified audio device\n");
  printf(" -echo         Echo audio input back to output\n");
  printf(" -showfps      Output frame rate to console\n");

  exit(0);
}

void Demo::openAudioInputFile() {
  SF_INFO sfinfo;
  if(!(audioInputFile = sf_open(audioInputFilename, SFM_READ, &sfinfo))) {
    printf("failed to open audio input file %s\n", audioInputFilename);
    exit(0);
  }
  if(sfinfo.samplerate != (int) audioParameters.sampleRate) {
    printf("expected sample rate %d\n", audioParameters.sampleRate);
    exit(0);
  }
  if(sfinfo.channels != 2) {
    printf("expected stereo audio\n");
    exit(0);
  }
  audioFileBuffer = new float [audioParameters.bufferSize * 2];
}

void Demo::initializeAudio() {
  PaError err;
  err = Pa_Initialize();
  if( err != paNoError ) {
    printf("failed to initialize portaudio\n");
    exit(0);
  }

  if(!useAudioInputFile) {
    printf("listening to audio device\n");
  }
}

static int Demo_portaudioCallback(
  const void *inputBuffer,
  void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo *timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData)
{
  Demo *demo = (Demo *)userData;
  return demo->audioCallback((float *) inputBuffer, (float *) outputBuffer, framesPerBuffer);
}

void Demo::openAudioStream() {
  int numInputChannels = 2;
  int numOutputChannels = echoAudio ? 2 : 0;
  int audioDeviceId = 0;
  PaStreamParameters outputParameters;
  PaStreamParameters inputParameters;
  PaError err;
  void *callbackUserData = (void *) this;

  const PaDeviceInfo *deviceInfo;
  int numDevices = Pa_GetDeviceCount();

  if(audioDeviceName != NULL) {
    for(int i=0; i<numDevices; i++) {
      deviceInfo = Pa_GetDeviceInfo(i);
      if(strncmp(audioDeviceName, deviceInfo->name, strlen(audioDeviceName)) == 0)
	audioDeviceId = i;
    }
  }
  for(int i=0; i<numDevices; i++) {
    deviceInfo = Pa_GetDeviceInfo(i);
    printf("device %d%s: '%s', %d in, %d out\n",
      i, (i == audioDeviceId) ? " SELECTED" : "",
      deviceInfo->name, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
  }

  bzero(&inputParameters, sizeof(inputParameters));
  inputParameters.channelCount = numInputChannels;
  inputParameters.device = audioDeviceId;
  inputParameters.sampleFormat = paFloat32;

  if(echoAudio) {
    bzero(&outputParameters, sizeof(outputParameters));
    outputParameters.channelCount = numOutputChannels;
    outputParameters.device = audioDeviceId;
    outputParameters.sampleFormat = paFloat32;
  }

  err = Pa_OpenStream(
    &paStream,
    &inputParameters,
    echoAudio ? &outputParameters : NULL,
    audioParameters.sampleRate,
    audioParameters.bufferSize,
    paNoFlag,
    Demo_portaudioCallback,
    callbackUserData
  );

  if( err != paNoError ) {
    printf("failed to open audio stream (%s)\n", Pa_GetErrorText(err));
    exit(0);
  }

  if(Pa_StartStream( paStream ) != paNoError ) {
    printf("failed to start stream\n");
    exit(0);
  }
}

void Demo::initializeAudioProcessing() {
  srand((unsigned) time(NULL));

  gridMap = new GridMap(audioParameters, gridMapParameters);
  spectrumAnalyzer = gridMap->getSpectrumAnalyzer();
  spectrumBinDivider = gridMap->getSpectrumBinDivider();
  spectrumMapInputBuffer = new float [audioParameters.bufferSize];
  circleMap = new CircleMap(audioParameters, circleMapParameters);
  beatTracker = new BeatTracker(spectrumBinDivider->getNumBins(), audioParameters.bufferSize, audioParameters.sampleRate);
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

void Demo::moveToScene(int _sceneNum) {
  sceneNum = _sceneNum;
  resizeFrames();
}

void Demo::initializeGraphics() {
  normalizeSpectrum = (spectrumAnalyzer->getPowerScale() == SpectrumAnalyzer::Amplitude);
  frameCount = 0;
  waveformFrame = new WaveformFrame(spectrumMapInputBuffer, audioParameters.bufferSize);
  spectrumFrame = new SpectrumFrame(spectrumAnalyzer, normalizeSpectrum);
  spectrumBinsFrame = new SpectrumBinsFrame(spectrumBinDivider, normalizeSpectrum);
  gridMapFrame = new GridMapFrame(gridMap);
  enlargedGridMapFrame = new SmoothGridMapFrame(gridMap);
  gridMapTrajectoryFrame = new GridMapTrajectoryFrame(gridMap);
  beatTrackerFrame = new BeatTrackerFrame(beatTracker);
  isolinesFrame = new IsolinesFrame(gridMap);
  circleMapFrame = new CircleMapFrame(circleMap, beatTracker);
  enlargedCircleMapFrame = new SmoothCircleMapFrame(circleMap);
  circleMapErrorPlotter = new ErrorPlotter(this, circleMap);
  gridMapErrorPlotter = new ErrorPlotter(this, gridMap);

  for(int i = 0; i < 20; i++)
    dancers.push_back(Dancer(this));

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);

  if(plotError)
    moveToScene(Scene_EnlargedCircleMap);
  else
    moveToScene(Scene_Mixed);
}

void Demo::glReshape(int _windowWidth, int _windowHeight) {
  windowWidth = _windowWidth;
  windowHeight = _windowHeight;
  glViewport (0, 0, (GLint) windowWidth - 1, (GLint) windowHeight - 1);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0.0, (GLdouble) windowWidth, (GLdouble) windowHeight, 0.0, -1.0, 1.0);
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
      circleMapFrame->setPosition(SPACING + displayWidth - columnWidth*2 - SPACING, y3);

      gridMapFrame->setSize(columnWidth, rowHeight);
      gridMapFrame->setPosition(SPACING + displayWidth - columnWidth, y3);
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

int Demo::audioCallback(float *inputBuffer, float *outputBuffer, unsigned long framesPerBuffer) {
  if(framesPerBuffer != audioParameters.bufferSize) {
    printf("expected %ld frames in audio callback, got %ld\n", audioParameters.bufferSize, framesPerBuffer);
    exit(0);
  }

  static float *inputPtr;
  if(useAudioInputFile) {
    readAudioBufferFromFile();
    inputPtr = audioFileBuffer;
  }
  else {
    inputPtr = inputBuffer;
  }

  float *spectrumMapInputBufferPtr = spectrumMapInputBuffer;
  float *outputPtr = (float *) outputBuffer;

  unsigned long i = 0;
  while(i < audioParameters.bufferSize) {
    *spectrumMapInputBufferPtr++ = *inputPtr;
    if(echoAudio) {
      *outputPtr++ = *inputPtr++;
      *outputPtr++ = *inputPtr++;
    }
    else {
      inputPtr += 2;
    }
    i++;
  }

  gridMap->feedAudio(spectrumMapInputBuffer, audioParameters.bufferSize);
  circleMap->feedAudio(spectrumMapInputBuffer, audioParameters.bufferSize);
  beatTracker->feedFeatureVector(spectrumBinDivider->getBinValues());

  if(plotError) {
    circleMapErrorPlotter->update();
    gridMapErrorPlotter->update();
  }

  return 0;
}

void Demo::readAudioBufferFromFile() {
  float *audioFileBufferPtr = audioFileBuffer;
  int framesLeft = audioParameters.bufferSize;
  while(framesLeft > 0) {
    int framesRead = sf_readf_float(audioInputFile, audioFileBufferPtr, framesLeft);
    if(framesRead < framesLeft)
      sf_seek(audioInputFile, 0, SEEK_SET); // rewind file
    framesLeft -= framesRead;
    audioFileBufferPtr += framesRead * 2;
  }
}

void Demo::glDisplay() {
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
      circleMapFrame->display();
      beatTrackerFrame->display();
      break;

    case Scene_Dancers:
      updateDancers();
      renderDancers();
      break;

    case Scene_EnlargedCircleMap:
      enlargedCircleMapFrame->display();
      if(plotError)
	circleMapErrorPlotter->render(enlargedCircleMapFrame);
      break;

    case Scene_EnlargedGridMap:
      enlargedGridMapFrame->display();
      if(plotError)
	gridMapErrorPlotter->render(enlargedGridMapFrame);
      break;

    case Scene_GridMapTrajectory:
      gridMapTrajectoryFrame->display();
      break;

    case Scene_Isolines:
      isolinesFrame->display();
      break;
  }

  glutSwapBuffers();

  frameCount++;

  if(showFPS) {
    if(frameCount % 100 == 0) {
      float FPS = (float)frameCount / stopwatch.getElapsedMilliseconds() * 1000;
      printf("fps=%.3f\n", FPS);
    }
  }

  if(showAdaptationValues) {
    if(frameCount % 50 == 0) {
      printf("grid   errorLevel=%.5f adaptationTimeSecs=%.5f neighbourhoodParameter=%.5f\n",
	     gridMap->getErrorLevel(),
	     gridMap->getAdaptationTimeSecs(),
	     gridMap->getNeighbourhoodParameter());
      printf("circle errorLevel=%.5f adaptationTimeSecs=%.5f neighbourhoodParameter=%.5f\n",
	     circleMap->getErrorLevel(),
	     circleMap->getAdaptationTimeSecs(),
	     circleMap->getNeighbourhoodParameter());
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

Demo::Dancer::Dancer(Demo *_parent) {
  parent = _parent;
  reset();
}

void Demo::Dancer::reset() {
  speed = 0;
  speedFactor = 0.3 + 0.3 * (float) rand() / RAND_MAX;
  length = 0.03;
  angle = 0;
  angleOffset = 2 * M_PI * (float) rand() / RAND_MAX;
  speedOffset = -(float) rand() / RAND_MAX * 0.2;
  trace.clear();
  currentPos.x = (float) rand() / RAND_MAX;
  currentPos.y = (float) rand() / RAND_MAX;
}

void Demo::Dancer::update(float timeIncrement) {
  angle = parent->circleMap->getAngle() + angleOffset;
  speed = (parent->beatTracker->getIntensity() + speedOffset) * speedFactor;

  float aspectRatio = (float) parent->windowHeight / parent->windowWidth;

  float distance = speed * timeIncrement;
  currentPos.x += cos(angle) * distance;
  currentPos.y += sin(angle) * distance / aspectRatio;
}

void Demo::Dancer::render() {
  updateTrace();
  renderTrace();
  if(traceOutOfBounds())
    reset();
}

void Demo::Dancer::updateTrace() {
  Point p;
  p.x = currentPos.x * parent->windowWidth;
  p.y = currentPos.y * parent->windowHeight;
  trace.push_back(p);
  if(trace.size() > 5)
    trace.erase(trace.begin());
}

void Demo::Dancer::renderTrace() {
  float c;
  glShadeModel(GL_SMOOTH);
  glLineWidth(2.0f);
  glBegin(GL_LINE_STRIP);
  vector<Point>::iterator pos = trace.begin();
  glColor3f(0, 0, 0);
  glVertex2f(pos->x, pos->y);
  pos++;
  int traceSize = trace.size();
  int n = 1;
  for(;pos != trace.end(); pos++) {
    c = (float) (n + 1) / traceSize;
    glColor3f(c, c, c);
    glVertex2f(pos->x, pos->y);
    n++;
  }
  glEnd();
}

bool Demo::Dancer::traceOutOfBounds() {
  return outOfBounds(*(trace.begin())) && outOfBounds(*(trace.end()));
}

bool Demo::Dancer::outOfBounds(const Point &p) {
  if(p.x < 0) return true;
  if(p.y < 0) return true;
  if(p.x > parent->windowWidth) return true;
  if(p.y > parent->windowHeight) return true;
  return false;
}

Demo::ErrorPlotter::ErrorPlotter(const Demo *parent, const SpectrumMap *_map) {
  map = _map;
  bufferSize = 1000;
  buffer = new float [bufferSize];
  circularBufferMin = new CircularBuffer<float> (bufferSize);
  circularBufferMax = new CircularBuffer<float> (bufferSize);
  maxValue = 0;
  maxValueInGraph = 0;
  
  float smootherResponseFactor = 1000 * parent->audioParameters.bufferSize
    / parent->audioParameters.sampleRate / map->getSpectrumMapParameters().errorIntegrationTimeMs;
  smootherMin.setResponseFactor(smootherResponseFactor);
  smootherMax.setResponseFactor(smootherResponseFactor);
}

Demo::ErrorPlotter::~ErrorPlotter() {
  delete [] buffer;
  delete circularBufferMin;
  delete circularBufferMax;
}

void Demo::ErrorPlotter::update() {
  circularBufferMin->write(smootherMin.smooth(map->getErrorMin()));
  circularBufferMin->moveReadHead(1);
  circularBufferMax->write(smootherMax.smooth(map->getErrorMax()));
  circularBufferMax->moveReadHead(1);
}

void Demo::ErrorPlotter::render(Frame *frame) {
  int i = 0;
  float y;
  int width = frame->getWidth();
  int height = frame->getHeight();

  circularBufferMax->read(bufferSize, buffer);
  for(int x = 0; x < width; x++) {
    i = (int) (bufferSize * x / width);
    y = buffer[i];
    if(y > maxValue) maxValue = y;
  }

  if(maxValue > (maxValueInGraph * 0.8))
    maxValueInGraph = maxValue * 1.5;

  if(maxValue > 0) {
    glShadeModel(GL_FLAT);
    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);
    for(int x = 0; x < width; x++) {
      i = (int) (bufferSize * x / width);
      y = buffer[i];
      frame->vertex2i(x, (int) ((1 - y / maxValueInGraph) * height));
    }
    glEnd();

    circularBufferMin->read(bufferSize, buffer);
    glBegin(GL_POINTS);
    glColor3f(0, 1, 0);
    for(int x = 0; x < width; x++) {
      i = (int) (bufferSize * x / width);
      y = buffer[i];
      frame->vertex2i(x, (int) ((1 - y / maxValueInGraph) * height));
    }
    glEnd();
  }
}

int main(int argc, char **argv) {
  Demo(argc, argv);
}
