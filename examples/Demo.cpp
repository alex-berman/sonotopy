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
  sonogramMapCircuitInputBuffer = NULL;
  audioDevice = 0;

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
  if(sonogramMapCircuitInputBuffer) delete sonogramMapCircuitInputBuffer;
}

void Demo::processCommandLineArguments() {
  useAudioInputFile = false;
  echoAudio = false;
  showFPS = false;
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
        audioDevice = atoi(*argptr);
      }
      else if(strcmp(argflag, "echo") == 0) {
        echoAudio = true;
      }
      else if(strcmp(argflag, "showfps") == 0) {
        showFPS = true;
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
  printf(" -d <N>        Use audio device number N\n");
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
  int numOutputChannels = 2;
  int inputDevice = audioDevice;
  int outputDevice = audioDevice;
  PaStreamParameters outputParameters;
  PaStreamParameters inputParameters;
  PaError err;
  void *callbackUserData = (void *) this;

  /* List all devices */
  const PaDeviceInfo *deviceInfo;
  int numDevices = Pa_GetDeviceCount();
  for(int i=0; i<numDevices; i++) {
    deviceInfo = Pa_GetDeviceInfo(i);
    printf("device %d%s: '%s', %d in, %d out\n",
      i, (i == audioDevice) ? " SELECTED" : "",
      deviceInfo->name, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
  }

  bzero(&inputParameters, sizeof(inputParameters));
  inputParameters.channelCount = numInputChannels;
  inputParameters.device = inputDevice;
  inputParameters.sampleFormat = paFloat32;

  bzero(&outputParameters, sizeof(outputParameters));
  outputParameters.channelCount = numOutputChannels;
  outputParameters.device = outputDevice;
  outputParameters.sampleFormat = paFloat32;

  err = Pa_OpenStream(
    &paStream,
    &inputParameters,
    &outputParameters,
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

  sonogramGridMapCircuit = new GridMapCircuit(audioParameters, gridMapCircuitParameters);
  sonogramGridMap = sonogramGridMapCircuit->getSonogramMap();
  spectrumAnalyzer = sonogramGridMapCircuit->getSpectrumAnalyzer();
  spectrumBinDivider = sonogramGridMapCircuit->getSpectrumBinDivider();
  gridTopology = (RectGridTopology*) sonogramGridMap->getTopology();
  sonogramGridMapWidth = gridTopology->getGridWidth();
  sonogramGridMapHeight = gridTopology->getGridHeight();
  sonogram = sonogramGridMapCircuit->getSonogram();
  sonogramMapCircuitInputBuffer = new float [audioParameters.bufferSize];

  sonogramCircleMapCircuit = new CircleMapCircuit(audioParameters, circleMapCircuitParameters);
  sonogramCircleMap = sonogramCircleMapCircuit->getSonogramMap();
  circleTopology = (CircleTopology*) sonogramCircleMap->getTopology();

  beatTracker = new BeatTracker(spectrumBinDivider->getNumBins(), audioParameters.bufferSize, audioParameters.sampleRate);
  vane = new Vane(audioParameters);
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
  sonogramFrame = new SonogramFrame(this);
  sonogramGridMapFrame = new SonogramGridMapFrame(this);
  enlargedSonogramGridMapFrame = new SmoothSonogramGridMapFrame(this);
  beatTrackerFrame = new BeatTrackerFrame(this);
  isolinesFrame = new IsolinesFrame(this);
  sonogramCircleMapFrame = new SonogramCircleMapFrame(this);
  enlargedSonogramCircleMapFrame = new SmoothSonogramCircleMapFrame(this);

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
  int beatTrackerWidth = columnWidth / 2;

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->setSize(displayWidth, rowHeight);
      waveformFrame->setPosition(SPACING, y0);

      spectrumFrame->setSize(displayWidth, rowHeight);
      spectrumFrame->setPosition(SPACING, y1);

      spectrumBinsFrame->setSize(displayWidth - beatTrackerWidth - SPACING, rowHeight);
      spectrumBinsFrame->setPosition(SPACING, y2);

      beatTrackerFrame->setSize(beatTrackerWidth, rowHeight);
      beatTrackerFrame->setPosition(SPACING + displayWidth - beatTrackerWidth, y2);

      sonogramFrame->setSize(columnWidth, rowHeight);
      sonogramFrame->setPosition(SPACING, y3);

      sonogramCircleMapFrame->setSize(columnWidth, rowHeight);
      sonogramCircleMapFrame->setPosition(SPACING + displayWidth - columnWidth*2 - SPACING, y3);

      sonogramGridMapFrame->setSize(columnWidth, rowHeight);
      sonogramGridMapFrame->setPosition(SPACING + displayWidth - columnWidth, y3);
      break;

    case Scene_EnlargedSonogramCircleMap:
      enlargedSonogramCircleMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedSonogramCircleMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_EnlargedSonogramGridMap:
      enlargedSonogramGridMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedSonogramGridMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
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

  float *sonogramMapCircuitInputBufferPtr = sonogramMapCircuitInputBuffer;
  float *outputPtr = (float *) outputBuffer;

  unsigned long i = 0;
  while(i < audioParameters.bufferSize) {
    *sonogramMapCircuitInputBufferPtr++ = *inputPtr;
    if(echoAudio) {
      *outputPtr++ = *inputPtr++;
      *outputPtr++ = *inputPtr++;
    }
    else {
      inputPtr += 2;
    }
    i++;
  }

  sonogramGridMapCircuit->feedAudio(sonogramMapCircuitInputBuffer, audioParameters.bufferSize);
  sonogramCircleMapCircuit->feedAudio(sonogramMapCircuitInputBuffer, audioParameters.bufferSize);
  vane->feedAudio(sonogramMapCircuitInputBuffer, audioParameters.bufferSize);
  beatTracker->feedFeatureVector(spectrumBinDivider->getBinValues());

  float timeIncrement = (float) audioParameters.bufferSize / audioParameters.sampleRate;
  updateVaneScene(timeIncrement);

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
  if(frameCount == 0)
    stopwatch.start();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  gridMapActivationPattern = sonogramGridMapCircuit->getActivationPattern();
  circleMapActivationPattern = sonogramCircleMapCircuit->getActivationPattern();

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->display();
      spectrumFrame->display();
      spectrumBinsFrame->display();
      sonogramFrame->display();
      sonogramGridMapFrame->display();
      sonogramCircleMapFrame->display();
      beatTrackerFrame->display();
      break;

    case Scene_Vane:
      renderVaneScene();
      break;

    case Scene_EnlargedSonogramCircleMap:
      enlargedSonogramCircleMapFrame->display();
      break;

    case Scene_EnlargedSonogramGridMap:
      enlargedSonogramGridMapFrame->display();
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
    x = parent->sonogramMapCircuitInputBuffer[(int) (parent->audioParameters.bufferSize * i / width)];
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

Demo::SonogramFrame::SonogramFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SonogramFrame::render() {
  const Sonogram::SonogramData *sonogramData = parent->sonogram->getSonogramData();
  Sonogram::SonogramData::Iterator sonogramDataIterator = sonogramData->begin();
  float w;
  int historyLength = parent->sonogram->getHistoryLength();
  int numBins = parent->sonogram->getSpectrumResolution();
  glShadeModel(GL_FLAT);
  for(int i = 0; i < historyLength; i++) {
    for(int j = 0; j < numBins; j++) {
      int x1, x2, py1, py2;
      x1 = (int) (i * width / historyLength);
      x2 = (int) ((i+1) * width /historyLength);
      py1 = (int) (j * height / numBins);
      py2 = (int) ((j+1) * height / numBins);
      w = *(sonogramDataIterator->value);
      if(parent->normalizeSpectrum) w = normalizer.normalize(w);
      glColor3f(w, w, w);
      glBegin(GL_POLYGON);
      vertex2i(x1, py1);
      vertex2i(x1, py2);
      vertex2i(x2, py2);
      vertex2i(x2, py1);
      vertex2i(x1, py1);
      glEnd();
      ++sonogramDataIterator;
    }
  }
}

Demo::SonogramGridMapFrame::SonogramGridMapFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SonogramGridMapFrame::render() {
  static float v;
  static int x1, x2, py1, py2;
  glShadeModel(GL_FLAT);
  SonogramMap::ActivationPattern::const_iterator activationPatternIterator = parent->gridMapActivationPattern->begin();
  for(int y = 0; y < parent->sonogramGridMapWidth; y++) {
    for(int x = 0; x < parent->sonogramGridMapHeight; x++) {
      v = *activationPatternIterator;
      glColor3f(v, v, v);
      glBegin(GL_POLYGON);
      x1 = (int) (width * x / parent->sonogramGridMapWidth);
      x2 = (int) (width * (x+1) / parent->sonogramGridMapWidth);
      py1 = (int) (y * height / parent->sonogramGridMapHeight);
      py2 = (int) ((y+1) * height / parent->sonogramGridMapHeight);
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

Demo::SmoothSonogramGridMapFrame::SmoothSonogramGridMapFrame(Demo *_parent) {
  parent = _parent;
}

void Demo::SmoothSonogramGridMapFrame::render() {
  static int x1, x2, py1, py2;
  glShadeModel(GL_SMOOTH);
  for(int y = 0; y < parent->sonogramGridMapWidth-1; y++) {
    for(int x = 0; x < parent->sonogramGridMapHeight-1; x++) {
      x1 = (int) (width * x / (parent->sonogramGridMapWidth-1));
      x2 = (int) (width * (x+1) / (parent->sonogramGridMapWidth-1));
      py1 = (int) (y * height / (parent->sonogramGridMapHeight-1));
      py2 = (int) ((y+1) * height / (parent->sonogramGridMapHeight-1));
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
}

void Demo::SmoothSonogramGridMapFrame::setColorFromActivationPattern(int x, int y) {
  float v = parent->sonogramGridMapCircuit->getActivation((unsigned int)x, (unsigned int)y);
  glColor3f(v, v, v);
}

Demo::SonogramCircleMapFrame::SonogramCircleMapFrame(Demo *_parent) {
  parent = _parent;
  numNodes = parent->circleTopology->getNumNodes();
}

void Demo::SonogramCircleMapFrame::render() {
  static float v, c;
  static int x1, y1, x2, y2;
  int centreX = width / 2;
  int centreY = height / 2;
  int radius = (int) (width * 0.4);
  float angleSpan = 2 * M_PI / numNodes;
  glShadeModel(GL_FLAT);
  SonogramMap::ActivationPattern::const_iterator activationPatternIterator = parent->circleMapActivationPattern->begin();
  CircleTopology::Node node;
  for(int i = 0; i < numNodes; i++) {
    v = *activationPatternIterator;
    c = 1.0f - v;
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

  float angle = parent->vane->getAngle();
  radius = parent->beatTracker->getIntensity() * width * 0.5;
  x1 = centreX + radius * cos(angle);
  y1 = centreY + radius * sin(angle);
  glColor3f(1.0f, 0.0f, 0.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  vertex2i(centreX, centreY);
  vertex2i(x1, y1);
  glEnd();
}

Demo::SmoothSonogramCircleMapFrame::SmoothSonogramCircleMapFrame(Demo *_parent) {
  parent = _parent;
  numNodes = parent->circleTopology->getNumNodes();
  angleIncrement = 2 * M_PI / numNodes;
}

void Demo::SmoothSonogramCircleMapFrame::render() {
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
}

float Demo::SmoothSonogramCircleMapFrame::getColorAtAngle(float angle) {
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

void Demo::updateVaneScene(float timeIncrement) {
  for(vector<Dancer>::iterator dancer = dancers.begin(); dancer != dancers.end(); dancer++)
    dancer->update(timeIncrement);
}

void Demo::renderVaneScene() {
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
  vaneAngle = 0;
  angleOffset = 2 * M_PI * (float) rand() / RAND_MAX;
  speedOffset = -(float) rand() / RAND_MAX * 0.2;
  trace.clear();
  currentPos.x = (float) rand() / RAND_MAX;
  currentPos.y = (float) rand() / RAND_MAX;
}

void Demo::Dancer::update(float timeIncrement) {
  angle = parent->vane->getAngle() + angleOffset;
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
  glBegin(GL_LINE_STRIP);
  glLineWidth(2.0f);
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
  activationPatternAsTwoDimArray = new TwoDimArray<float>(parent->sonogramGridMapWidth, parent->sonogramGridMapHeight);
  isolineExtractor = new IsolineExtractor(parent->sonogramGridMapWidth, parent->sonogramGridMapHeight);
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
        px = (int) (width * cx / (parent->sonogramGridMapWidth-1));
        py = (int) (height * cy / (parent->sonogramGridMapHeight-1));
        vertex2i(px, py);
      }
      glEnd();
    }
    ip++;
  }
}

int main(int argc, char **argv) {
  Demo(argc, argv);
}
