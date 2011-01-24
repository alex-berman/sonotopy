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

#include "performanceTest.hpp"
#include <string.h>
#include <stdlib.h>

using namespace sonotopy;

PerformanceTest::PerformanceTest(int _argc, char **_argv) {
  argc = _argc;
  argv = _argv;
  audioInputFile = NULL;
  audioFileBuffer = NULL;
  gridMapCircuitAudioInputBuffer = NULL;

  processCommandLineArguments();
  openAudioInputFile();
  initializeAudioProcessing();
  startStopwatch();
  performTestIterations();
  outputMeasuredTime();
}

PerformanceTest::~PerformanceTest() {
  if(audioInputFile) sf_close(audioInputFile);
  if(audioFileBuffer) delete audioFileBuffer;
  if(gridMapCircuitAudioInputBuffer) delete gridMapCircuitAudioInputBuffer;
}

void PerformanceTest::processCommandLineArguments() {
  numTestTypes = 0;
  numIterations = 1;
  testSpectrumMap = false;
  audioInputFilename = NULL;
  int argnr = 1;
  char **argptr = argv + 1;
  char *arg;
  while(argnr < argc) {
    arg = *argptr;
    if(arg[0] == '-') {
      char *argflag = arg + 1;
      if(strcmp(argflag, "f") == 0) {
        argnr++; argptr++;
        audioInputFilename = *argptr;
      }
      else if(strcmp(argflag, "sm") == 0) {
        testSpectrumMap = true;
        numTestTypes++;
      }
      else if(strcmp(argflag, "n") == 0) {
        argnr++; argptr++;
        numIterations = atoi(*argptr);
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

  if(audioInputFilename == NULL) {
    printf("Please specify an audio input file\n");
    usage();
  }
  if(numTestTypes == 0) {
    printf("Please specify at least one type of test\n");
    usage();
  }
}

void PerformanceTest::usage() {
  printf("Usage: %s [options]\n\n", argv[0]);

  printf("Options:\n\n");

  printf(" -sm           Test spectrum map\n");
  printf(" -f <WAV file> Use audio file as input\n");
  printf(" -n <N>        Run N number of iterations\n");

  exit(0);
}

void PerformanceTest::openAudioInputFile() {
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

void PerformanceTest::initializeAudioProcessing() {
  if(testSpectrumMap) {
    gridMapCircuit = new GridMapCircuit(audioParameters, gridMapCircuitParameters);
    gridMapCircuitAudioInputBuffer = new float [audioParameters.bufferSize];
  }
}

void PerformanceTest::processAudioBuffer() {
  float *inputPtr = audioFileBuffer;
  float *gridMapCircuitAudioInputBufferPtr = gridMapCircuitAudioInputBuffer;

  unsigned long i = 0;
  while(i < audioParameters.bufferSize) {
    *gridMapCircuitAudioInputBufferPtr++ = *inputPtr;
    inputPtr += 2;
    i++;
  }

  gridMapCircuit->feedAudio(gridMapCircuitAudioInputBuffer, audioParameters.bufferSize);
  activationPattern = gridMapCircuit->getActivationPattern();
}

void PerformanceTest::readAudioBufferFromFile() {
  unsigned int framesRead = (unsigned int) sf_readf_float(audioInputFile, audioFileBuffer, audioParameters.bufferSize);
  if(framesRead < audioParameters.bufferSize) {
    audioFileAtEnd = true;
    int framesLeftInBuffer = audioParameters.bufferSize - framesRead;
    memset(audioFileBuffer + framesRead*2, 0, sizeof(float) * 2 * framesLeftInBuffer);
  }
}

void PerformanceTest::performTestIterations() {
  for(int i = 0; i < numIterations; i++)
    performTestIteration();
}

void PerformanceTest::performTestIteration() {
  rewindAudioFile();
  while(!audioFileAtEnd) {
    readAudioBufferFromFile();
    processAudioBuffer();
  }
}

void PerformanceTest::rewindAudioFile() {
  sf_seek(audioInputFile, 0, SEEK_SET);
  audioFileAtEnd = false;
}

void PerformanceTest::startStopwatch() {
  stopwatch.start();
}

void PerformanceTest::outputMeasuredTime() {
  unsigned long elapsedMilliseconds = stopwatch.getElapsedMilliseconds();
  float elapsedSeconds = (float) elapsedMilliseconds / 1000;
  printf("test completed in %.2f seconds\n", elapsedSeconds);
}

int main(int argc, char **argv) {
  PerformanceTest(argc, argv);
}
