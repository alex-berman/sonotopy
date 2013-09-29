// Copyright (C) 2013 Alexander Berman
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

Demo::Demo(int _argc, char **_argv) :
  GlWindow(_argc, _argv),
  AudioIO()
{
  argc = _argc;
  argv = _argv;
  processCommandLineArguments();
}

Demo::~Demo() {
}

void Demo::runDemo() {
  frameCount = 0;
  pthread_mutex_init(&mutex, NULL);
  initializeAudio();
  initializeGraphics();
  pretrain();
  openAudioStream();
  glutMainLoop();
}

void Demo::processCommandLineArguments() {
  parser.add<string>("audiofile", 'f', "Audio file for input", false);
  parser.add<int>("bufferSize", 'b', "Audio buffer size", false, audioParameters.bufferSize);
  parser.add<string>("audiodevice", 'd', "Audio device", false);
  parser.add("echo", '\0', "Echo audio input back to output");
  parser.add("showfps", '\0', "Output frame rate to console");
  parser.add<float>("pretrain", '\0', "Pre-train for N seconds", false, 0.0);
  parser.add<int>("width", 'w', "Window width", false, 800);
  parser.add<int>("height", 'h', "Window height", false, 600);
  parser.add("export", '\0', "Export video");
  parser.add<int>("gridMapWidth", '\0', "Grid map width", false, gridMapParameters.gridWidth);
  parser.add<int>("gridMapHeight", '\0', "Grid map height", false, gridMapParameters.gridHeight);
  parser.add<int>("windowSize", '\0', "Spectrum analyzer window size (2^N)", false,
		  spectrumAnalyzerParameters.windowSize);
  parser.add<float>("windowOverlap", '\0', "Spectrum analyzer window overlap (0-1)", false,
		    spectrumAnalyzerParameters.windowOverlap);
  ColorScheme::addParserArguments(parser);
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
  audioParameters.bufferSize = parser.get<int>("bufferSize");
  gridMapParameters.gridWidth = parser.get<int>("gridMapWidth");
  gridMapParameters.gridHeight = parser.get<int>("gridMapHeight");
  spectrumAnalyzerParameters.windowSize = parser.get<int>("windowSize");
  spectrumAnalyzerParameters.windowOverlap = parser.get<float>("windowOverlap");

  if(parser.exist("export")) {
    audioEnableVideoExport();
    windowEnableVideoExport();
  }
}

void Demo::glKeyboard(unsigned char key, int x, int y) {
  switch(key) {
  case 27: // escape
    exit(0);
  }
}

void Demo::initializeGraphics() {
  setWindowSize(parser.get<int>("width"), parser.get<int>("height"));
  colorScheme = ColorScheme::createFromParser(parser);
  GlWindow::initializeGraphics();
}

void Demo::processAudio(float *inputBuffer) {
  pthread_mutex_lock(&mutex);
  processDemoAudio(inputBuffer);
  pthread_mutex_unlock(&mutex);
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

  if(exportEnabled) {
    readAudioBufferFromFile();
    processDemoAudio(monauralInputBuffer);
  }

  renderDemoGraphics();

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

void Demo::pretrain() {
  int pretrainBuffers = parser.get<float>("pretrain") * audioParameters.sampleRate /
    audioParameters.bufferSize;
  if(pretrainBuffers > 0) {
    printf("pre-training...\n");
    for(int i = 0; i < pretrainBuffers; i++) {
      readAudioBufferFromFile();
      processDemoAudio(audioFileBuffer);
    }
    rewindAudioInputFile();
    printf("ok\n");
  }
}
