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
#include <sndfile.h>

using namespace sonotopy;

class PerformanceTest {
public:
  PerformanceTest(int _argc, char **_argv);
  ~PerformanceTest();

private:
  void processCommandLineArguments();
  void usage();
  void openAudioInputFile();
  void rewindAudioFile();
  void initializeAudioProcessing();
  void performTestIterations();
  void performTestIteration();
  void processAudioBuffer();
  void readAudioBufferFromFile();
  void startStopwatch();
  void outputMeasuredTime();

  int argc;
  char **argv;
  char *audioInputFilename;
  int numTestTypes;
  int numIterations;
  bool testSonogramMap;
  bool audioFileAtEnd;
  AudioParameters audioParameters;
  GridMapCircuitParameters sonogramGridMapCircuitParameters;
  GridMapCircuit *sonogramGridMapCircuit;
  float *sonogramGridMapCircuitAudioInputBuffer;
  SNDFILE *audioInputFile;
  float *audioFileBuffer;
  SonogramMap::ActivationPattern *activationPattern;
  Stopwatch stopwatch;
};
