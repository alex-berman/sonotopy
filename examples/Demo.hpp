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

#include <sonotopy/sonotopy.hpp>
#include <sonotopy/uilib/uilib.hpp>
#include <sonotopy/uilib/cmdline.hpp>
#include <sonotopy/uilib/ColorScheme.hpp>
#include <pthread.h>

class Demo : public GlWindow, public AudioIO {
public:
  Demo(int _argc, char **_argv);
  ~Demo();
  void processAudio(float *);
  void display();
  void glKeyboard(unsigned char key, int x, int y);
  virtual void renderDemoGraphics()=0;
  virtual void processDemoAudio(float *)=0;
  virtual void initializeGraphics();
  void runDemo();

protected:
  void processCommandLineArguments();
  void pretrain();

  cmdline::parser parser;
  SpectrumAnalyzerParameters spectrumAnalyzerParameters;
  GridMapParameters gridMapParameters;
  Stopwatch stopwatch;
  int argc;
  char **argv;
  bool showFPS;
  unsigned long displayStartTime;
  unsigned long frameCount;
  float timeOfPreviousDisplay;
  float timeIncrement;
  pthread_mutex_t mutex;
  ColorScheme *colorScheme;
};
