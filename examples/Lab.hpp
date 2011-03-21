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
#include "GlWindow.hpp"
#include "AudioIO.hpp"
#include "SmoothGridMapFrame.hpp"
#include <pthread.h>

class Lab : public GlWindow, public AudioIO {
public:
  Lab(int _argc, char **_argv);
  void processAudio(float *);
  void display();
  void resizedWindow();

private:
  class ErrorPlotter {
  public:
    ErrorPlotter(Lab *, const SpectrumMap *);
    ~ErrorPlotter();
    void update();
    void render(Frame *);
  private:
    Lab *parent;
    const SpectrumMap *map;
    Frame *frame;
    float *buffer;
    CircularBuffer<float> *circularBufferMin;
    CircularBuffer<float> *circularBufferMax;
    int bufferSize;
    float maxValue;
    float maxValueInGraph;
    Smoother smootherMin, smootherMax;
  };

  class ComparedMap {
  public:
    ComparedMap(Lab *, GridMapParameters &);
    void initializeGraphics();
    void display();
    void processAudio(float *buffer, unsigned long numFrames);
    Frame *getFrame() { return gridMapFrame; }
  private:
    Lab *parent;
    GridMapParameters parameters;
    GridMap *gridMap;
    SmoothGridMapFrame *gridMapFrame;
    ErrorPlotter *errorPlotter;
  };

  void processCommandLineArguments();
  void usage();
  void initializeAudioProcessing();
  void initializeGraphics();
  void addGridMap();

  int argc;
  char **argv;
  GridMapParameters gridMapParameters;
  std::vector<ComparedMap> comparedMaps;
  bool showAdaptationValues;
  bool plotError;
  pthread_mutex_t mutex;
};
