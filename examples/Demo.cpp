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

const float Demo::activationPatternContrast = 15.0f;

Demo::Demo(int _argc, char **_argv) : GlWindow(_argc, _argv, 800, 600) {
  demo = this;
  argc = _argc;
  argv = _argv;
  audioFileBuffer = NULL;
  spectrumMapCircuitInputBuffer = NULL;
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
  if(spectrumMapCircuitInputBuffer) delete spectrumMapCircuitInputBuffer;
}

void Demo::processCommandLineArguments() {
  useAudioInputFile = false;
  echoAudio = false;
  showFPS = false;
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

  gridMapCircuit = new GridMapCircuit(audioParameters, gridMapCircuitParameters);
  gridMap = gridMapCircuit->getSpectrumMap();
  spectrumAnalyzer = gridMapCircuit->getSpectrumAnalyzer();
  spectrumBinDivider = gridMapCircuit->getSpectrumBinDivider();
  gridMapWidth = gridMapCircuitParameters.gridWidth;
  gridMapHeight = gridMapCircuitParameters.gridHeight;
  spectrumMapCircuitInputBuffer = new float [audioParameters.bufferSize];

  circleMapCircuit = new CircleMapCircuit(audioParameters, circleMapCircuitParameters);
  circleMap = circleMapCircuit->getSpectrumMap();
  circleTopology = (CircleTopology*) circleMap->getTopology();

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
  sceneNum = 0;
  frameCount = 0;
  waveformFrame = new WaveformFrame(this);
  spectrumFrame = new SpectrumFrame(this);
  spectrumBinsFrame = new SpectrumBinsFrame(this);
  gridMapFrame = new GridMapFrame(this);
  enlargedGridMapFrame = new SmoothGridMapFrame(this);
  gridMapTrajectoryFrame = new GridMapTrajectoryFrame(this);
  beatTrackerFrame = new BeatTrackerFrame(this);
  isolinesFrame = new IsolinesFrame(this);
  circleMapFrame = new CircleMapFrame(this);
  enlargedCircleMapFrame = new SmoothCircleMapFrame(this);
  circleMapErrorPlotter = new ErrorPlotter(circleMap);
  gridMapErrorPlotter = new ErrorPlotter(gridMap);

  for(int i = 0; i < 20; i++)
    dancers.push_back(Dancer(this));

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);
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

  float *spectrumMapCircuitInputBufferPtr = spectrumMapCircuitInputBuffer;
  float *outputPtr = (float *) outputBuffer;

  unsigned long i = 0;
  while(i < audioParameters.bufferSize) {
    *spectrumMapCircuitInputBufferPtr++ = *inputPtr;
    if(echoAudio) {
      *outputPtr++ = *inputPtr++;
      *outputPtr++ = *inputPtr++;
    }
    else {
      inputPtr += 2;
    }
    i++;
  }

  gridMapCircuit->feedAudio(spectrumMapCircuitInputBuffer, audioParameters.bufferSize);
  circleMapCircuit->feedAudio(spectrumMapCircuitInputBuffer, audioParameters.bufferSize);
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

  gridMapActivationPattern = gridMapCircuit->getActivationPattern();
  circleMapActivationPattern = circleMapCircuit->getActivationPattern();

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

  glutSwapBuffers();

  frameCount++;

  if(showFPS) {
    if(frameCount%100==0) {
      float FPS = (float)frameCount / stopwatch.getElapsedMilliseconds() * 1000;
      printf("fps=%.3f\n", FPS);
    }
  }
}

Demo::WaveformFrame::WaveformFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::WaveformFrame::render() {
  glColor3f(1.0f, 1.0f, 1.0f);
  static float x;
  glShadeModel(GL_FLAT);
  glBegin(GL_POINTS);
  for(int i = 0; i < width; i++) {
    x = parent->spectrumMapCircuitInputBuffer[(int) (parent->audioParameters.bufferSize * i / width)];
    vertex2i(i, (int) ((x + 1) / 2 * height));
  }
  glEnd();
}

Demo::SpectrumFrame::SpectrumFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SpectrumFrame::render() {
  const float *spectrum = parent->spectrumAnalyzer->getSpectrum();
  static float z;
  static int spectrumBin;
  glShadeModel(GL_FLAT);
  glLineWidth(1.0f);
  for(int i = 0; i < width; i++) {
    spectrumBin = (int) (parent->spectrumAnalyzer->getSpectrumResolution() * i / width);
    z = spectrum[spectrumBin];
    if(parent->normalizeSpectrum) z = normalizer.normalize(log(1.0f + z));
    glColor3f(z, z, z);
    glBegin(GL_LINES);
    vertex2i(i, height);
    vertex2i(i, height - (int) (z * height));
    glEnd();
  }
}

Demo::SpectrumBinsFrame::SpectrumBinsFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SpectrumBinsFrame::render() {
  const float *binValuePtr = parent->spectrumBinDivider->getBinValues();
  static float w;
  static int x1, x2, y1, y2;
  unsigned int numBins = parent->spectrumBinDivider->getNumBins();
  glShadeModel(GL_FLAT);
  y1 = height;
  for(unsigned int i = 0; i < numBins; i++) {
    w = *binValuePtr;
    if(parent->normalizeSpectrum) w = normalizer.normalize(w);
    glColor3f(w, w, w);
    glBegin(GL_POLYGON);
    x1 = (int) (width * i / numBins);
    x2 = (int) (width * (i+1) / numBins);
    y2 = height - (int) (height * w);
    vertex2i(x1, y1);
    vertex2i(x1, y2);
    vertex2i(x2, y2);
    vertex2i(x2, y1);
    vertex2i(x1, y1);
    glEnd();
    binValuePtr++;
  }
}

Demo::GridMapFrame::GridMapFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::GridMapFrame::render() {
  glShadeModel(GL_FLAT);
  renderActivationPattern();
  renderCursor();
}

void Demo::GridMapFrame::renderActivationPattern() {
  static float v;
  static int x1, x2, py1, py2;
  SpectrumMap::ActivationPattern::const_iterator activationPatternIterator =
    parent->gridMapActivationPattern->begin();
  for(int y = 0; y < parent->gridMapWidth; y++) {
    for(int x = 0; x < parent->gridMapHeight; x++) {
      v = *activationPatternIterator;
      v = pow(v, activationPatternContrast);
      glColor3f(v, v, v);
      glBegin(GL_POLYGON);
      x1 = (int) (width * x / parent->gridMapWidth);
      x2 = (int) (width * (x+1) / parent->gridMapWidth);
      py1 = (int) (y * height / parent->gridMapHeight);
      py2 = (int) ((y+1) * height / parent->gridMapHeight);
      vertex2i(x1, py1);
      vertex2i(x1, py2);
      vertex2i(x2, py2);
      vertex2i(x2, py1);
      vertex2i(x1, py1);
      glEnd();
      activationPatternIterator++;
    }
  }
}

void Demo::GridMapFrame::renderCursor() {
  float wx, wy;
  int x1, y1, x2, y2;
  int s = (int) (width / parent->gridMapWidth / 2);
  parent->gridMapCircuit->getCursor(wx, wy);
  x1 = (int) (width  * wx) - s;
  y1 = (int) (height * wy) - s;
  x2 = (int) (width  * wx) + s;
  y2 = (int) (height * wy) + s;
  glColor3f(1, 0, 0);
  glBegin(GL_POLYGON);
  vertex2i(x1, y1);
  vertex2i(x2, y1);
  vertex2i(x2, y2);
  vertex2i(x1, y2);
  vertex2i(x1, y1);
  glEnd();
}

Demo::SmoothGridMapFrame::SmoothGridMapFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SmoothGridMapFrame::render() {
  static int x1, x2, py1, py2;
  glShadeModel(GL_SMOOTH);
  for(int y = 0; y < parent->gridMapWidth-1; y++) {
    for(int x = 0; x < parent->gridMapHeight-1; x++) {
      x1 = (int) (width * x / (parent->gridMapWidth-1));
      x2 = (int) (width * (x+1) / (parent->gridMapWidth-1));
      py1 = (int) (y * height / (parent->gridMapHeight-1));
      py2 = (int) ((y+1) * height / (parent->gridMapHeight-1));
      glBegin(GL_POLYGON);
      setColorFromActivationPattern(x, y);
      vertex2i(x1, py1);
      setColorFromActivationPattern(x, y+1);
      vertex2i(x1, py2);
      setColorFromActivationPattern(x+1, y+1);
      vertex2i(x2, py2);
      setColorFromActivationPattern(x+1, y);
      vertex2i(x2, py1);
      setColorFromActivationPattern(x, y);
      vertex2i(x1, py1);
      glEnd();
    }
  }

  if(parent->plotError)
    parent->gridMapErrorPlotter->render(this);
}

Demo::GridMapTrajectoryFrame::GridMapTrajectoryFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::GridMapTrajectoryFrame::render() {
  updateTrace();
  renderTrace();
}

void Demo::GridMapTrajectoryFrame::updateTrace() {
  Point p;
  float wx, wy;
  parent->gridMapCircuit->getCursor(wx, wy);
  p.x = wx * width;
  p.y = wy * height;
  trace.push_back(p);
  if(trace.size() > 10)
    trace.erase(trace.begin());
}

void Demo::GridMapTrajectoryFrame::renderTrace() {
  float c;
  glShadeModel(GL_SMOOTH);
  glLineWidth(3.0f);
  glBegin(GL_LINE_STRIP);
  vector<Point>::iterator pos = trace.begin();
  glColor3f(0, 0, 0);
  vertex2f(pos->x, pos->y);
  pos++;
  int traceSize = trace.size();
  int n = 1;
  for(;pos != trace.end(); pos++) {
    c = (float) (n + 1) / traceSize;
    glColor3f(c, c, c);
    vertex2f(pos->x, pos->y);
    n++;
  }
  glEnd();
}

void Demo::SmoothGridMapFrame::setColorFromActivationPattern(int x, int y) {
  float v = parent->gridMapCircuit->getActivation((unsigned int)x, (unsigned int)y);
  v = pow(v, activationPatternContrast);
  glColor3f(v, v, v);
}

Demo::CircleMapFrame::CircleMapFrame(Demo *_parent) {
  parent = _parent;
  numNodes = parent->circleTopology->getNumNodes();
}

void Demo::CircleMapFrame::render() {
  static float c;
  static int x1, y1, x2, y2;
  int centreX = width / 2;
  int centreY = height / 2;
  int radius = (int) (width * 0.4);
  float angleSpan = 2 * M_PI / numNodes;
  glShadeModel(GL_FLAT);
  SpectrumMap::ActivationPattern::const_iterator activationPatternIterator = parent->circleMapActivationPattern->begin();
  CircleTopology::Node node;
  for(int i = 0; i < numNodes; i++) {
    c = *activationPatternIterator;
    node = parent->circleTopology->getNode(i);
    x1 = centreX + radius * cos(node.angle - angleSpan);
    y1 = centreY + radius * sin(node.angle - angleSpan);
    x2 = centreX + radius * cos(node.angle + angleSpan);
    y2 = centreY + radius * sin(node.angle + angleSpan);
    glBegin(GL_POLYGON);
    glColor3f(c, c, c);
    vertex2i(centreX, centreY);
    vertex2i(x1, y1);
    vertex2i(x2, y2);
    glEnd();
    activationPatternIterator++;
  }

  float angle = parent->circleMapCircuit->getAngle();
  radius = width * (0.1 + parent->beatTracker->getIntensity() * 0.3);
  x1 = centreX + radius * cos(angle);
  y1 = centreY + radius * sin(angle);
  glColor3f(1.0f, 0.0f, 0.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  vertex2i(centreX, centreY);
  vertex2i(x1, y1);
  glEnd();
}

Demo::SmoothCircleMapFrame::SmoothCircleMapFrame(Demo *_parent) {
  parent = _parent;
  numNodes = parent->circleTopology->getNumNodes();
  angleIncrement = 2 * M_PI / numNodes;
}

void Demo::SmoothCircleMapFrame::render() {
  int numPoints = 100;
  float c;
  float a;
  int x, y;
  int centreX = width / 2;
  int centreY = height / 2;
  int radius = (int) (width * 0.4);

  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(1,1,1);
  vertex2i(centreX, centreY);
  for(int i = 0; i <= numPoints; i++) {
    a = (float) i / numPoints * 2 * M_PI;
    c = getColorAtAngle(a);
    glColor3f(c, c, c);
    x  = centreX + radius * cos(a);
    y  = centreY + radius * sin(a);
    vertex2i(x, y);
  }
  glEnd();

  if(parent->plotError)
    parent->circleMapErrorPlotter->render(this);
}

float Demo::SmoothCircleMapFrame::getColorAtAngle(float angle) {
  angle = fmodf(angle, 2 * M_PI);
  int nodeId1 = (int) (angle / angleIncrement);
  int nodeId2 = (nodeId1 + 1) % numNodes;
  float nodeAngle1 = (float) nodeId1 / numNodes * 2 * M_PI;
  float nodeStrength1 = 1.0f - (angle - nodeAngle1) / angleIncrement;
  float nodeStrength2 = 1.0f - nodeStrength1;
  float nodeActivity1 = (*parent->circleMapActivationPattern)[nodeId1];
  float nodeActivity2 = (*parent->circleMapActivationPattern)[nodeId2];
  return nodeActivity1 * nodeStrength1 + nodeActivity2 * nodeStrength2;
}

Demo::BeatTrackerFrame::BeatTrackerFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::BeatTrackerFrame::render() {
  float i = parent->beatTracker->getIntensity();
  int x = (1-i) * width / 2;
  int y = (1-i) * height / 2;
  glColor3f(i, i, i);
  glBegin(GL_POLYGON);
  vertex2i(x, y);
  vertex2i(width-x, y);
  vertex2i(width-x, height-y);
  vertex2i(x, height-y);
  vertex2i(x, y);
  glEnd();
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
  angle = parent->circleMapCircuit->getAngle() + angleOffset;
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

Demo::IsolinesFrame::IsolinesFrame(Demo *_parent) {
  parent = _parent;
  activationPatternAsTwoDimArray = new TwoDimArray<float>(parent->gridMapWidth, parent->gridMapHeight);
  isolineExtractor = new IsolineExtractor(parent->gridMapWidth, parent->gridMapHeight);
  isolineRenderer = new IsolineRenderer(isolineExtractor);
  lineWidthFactor = 0.1f;
  isocurvesHistoryLength = 7;
  isocurvesHistoryCurrentLength = 0;
}

void Demo::IsolinesFrame::render() {
  static IsolineRenderer::DrawableIsocurveSet drawableIsocurveSet;
  activationPatternToTwoDimArray();
  isolineExtractor->setMap(*activationPatternAsTwoDimArray);
  isolineRenderer->getDrawableIsocurveSet(drawableIsocurveSet);
  addDrawableIsocurveSetToHistory(drawableIsocurveSet);
  renderDrawableIsocurveSetHistory();
}

void Demo::IsolinesFrame::activationPatternToTwoDimArray() {
  TwoDimArray<float>::Iterator twoDimArrayIterator = activationPatternAsTwoDimArray->begin();
  for(vector<float>::const_iterator vectorIterator = parent->gridMapActivationPattern->begin();
    vectorIterator != parent->gridMapActivationPattern->end();
    vectorIterator++)
  {
    *(twoDimArrayIterator->value) = *vectorIterator;
    twoDimArrayIterator++;
  }
}

void Demo::IsolinesFrame::addDrawableIsocurveSetToHistory(const IsolineRenderer::DrawableIsocurveSet &drawableIsocurveSet) {
  isocurvesHistory.push_back(drawableIsocurveSet);
  if(isocurvesHistoryCurrentLength == isocurvesHistoryLength)
    isocurvesHistory.erase(isocurvesHistory.begin());
  else
    isocurvesHistoryCurrentLength++;
}

void Demo::IsolinesFrame::renderDrawableIsocurveSetHistory() {
  static float is, ic;
  static float ilMin, ilMax;
  static float cx, cy;
  static int px, py;
  ilMin = demo->getWindowWidth() * 0.003f;
  ilMax = demo->getWindowWidth() * 0.010f * (lineWidthFactor / 0.1f);
  static vector<IsolineRenderer::DrawableIsocurveSet>::iterator ip;
  ip = isocurvesHistory.begin();
  for(int i = 0; i < isocurvesHistoryCurrentLength; i++) {
    is = (float) i / (isocurvesHistoryCurrentLength - 1);
    ic = powf(is, 2.0f);
    glColor3f(ic, ic, ic);
    glLineWidth (ilMin + ilMax * (1.0f-is));
    for(IsolineExtractor::CurveSet::iterator curve = ip->curves.begin(); curve != ip->curves.end(); curve++) {
      glBegin(GL_LINE_STRIP);
      for(vector<IsolineExtractor::Point>::iterator v = curve->points.begin(); v != curve->points.end(); v++) {
        cx = v->x;
        cy = v->y;
        px = (int) (width * cx / (parent->gridMapWidth-1));
        py = (int) (height * cy / (parent->gridMapHeight-1));
        vertex2i(px, py);
      }
      glEnd();
    }
    ip++;
  }
}

Demo::ErrorPlotter::ErrorPlotter(const SpectrumMap *_map) {
  map = _map;
  bufferSize = 1000;
  buffer = new float [bufferSize];
  circularBufferMin = new CircularBuffer<float> (bufferSize);
  circularBufferMax = new CircularBuffer<float> (bufferSize);
  maxValue = 0;
  smootherMin.setResponseFactor(0.01);
  smootherMax.setResponseFactor(0.01);
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

  if(maxValue > 0) {
    glShadeModel(GL_FLAT);
    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);
    for(int x = 0; x < width; x++) {
      i = (int) (bufferSize * x / width);
      y = buffer[i];
      frame->vertex2i(x, (int) ((1 - y / maxValue) * height));
    }
    glEnd();

    circularBufferMin->read(bufferSize, buffer);
    glBegin(GL_POINTS);
    glColor3f(0, 1, 0);
    for(int x = 0; x < width; x++) {
      i = (int) (bufferSize * x / width);
      y = buffer[i];
      frame->vertex2i(x, (int) ((1 - y / maxValue) * height));
    }
    glEnd();
  }
}

int main(int argc, char **argv) {
  Demo(argc, argv);
}
