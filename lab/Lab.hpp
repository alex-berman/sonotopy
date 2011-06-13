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
#include <sonotopy/uilib/uilib.hpp>
#include <pthread.h>
#include <sstream>
#include <fstream>

class Lab : public GlWindow, public AudioIO {
public:
  Lab(int _argc, char **_argv);
  void processAudio(float *);
  void display();
  void resizedWindow();
  void glKeyboard(unsigned char key, int x, int y);

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
    ComparedMap(Lab *, int index);
    virtual void initializeGraphics() {}
    virtual void display() {}
    virtual void processAudio(float *buffer, unsigned long numFrames) {}
    Frame *getFrame() { return frame; }
    void generatePlotFile();
    virtual void writePlotFilesContent() {}
  protected:
    int index;
    Lab *parent;
    Frame *frame;
    ErrorPlotter *errorPlotter;
    std::string dataFilename;
    std::string plotFilename;
    std::ofstream dataFile;
    std::ofstream plotFile;
  };

  class ComparedGridMap : public ComparedMap {
  public:
    ComparedGridMap(Lab *, int index, GridMapParameters &);
    void initializeGraphics();
    void display();
    void processAudio(float *buffer, unsigned long numFrames);
    void writePlotFilesContent();
  private:
    GridMapParameters parameters;
    GridMap *gridMap;
  };

  class ComparedCircleMap : public ComparedMap {
  public:
    ComparedCircleMap(Lab *, int index, CircleMapParameters &);
    void initializeGraphics();
    void display();
    void processAudio(float *buffer, unsigned long numFrames);
    void writePlotFilesContent();
  private:
    CircleMapParameters parameters;
    CircleMap *circleMap;
    CircleTopology *topology;
  };

  void processCommandLineArguments();
  void usage();
  void initializeAudioProcessing();
  void initializeGraphics();
  void addGridMap();
  void addCircleMap();
  void addComparedMap(ComparedMap *map);
  void generatePlotFiles();

  int argc;
  char **argv;
  GridMapParameters gridMapParameters;
  CircleMapParameters circleMapParameters;
  std::vector<ComparedMap*> comparedMaps;
  bool plotError;
  pthread_mutex_t mutex;
  unsigned long t0;
  int mapCount;
  int plotFileCount;
};
