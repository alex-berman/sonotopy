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
  class ErrorGraph {
  public:
    ErrorGraph(Lab *, const SpectrumMap *);
    ~ErrorGraph();
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
    int getIndex() { return index; }
    virtual void initializeGraphics() {}
    virtual void display() {}
    virtual void processAudio(float *buffer, unsigned long numFrames) {}
    Frame *getFrame() { return frame; }
    virtual void generatePlotFiles();
    virtual void writePlotFilesContent() {}
    virtual void startTrajectoryPlotting() {}
    virtual void stopTrajectoryPlotting() {}
    virtual void plotTrajectory() {}
  protected:
    int index;
    Lab *parent;
    Frame *frame;
    ErrorGraph *errorGraph;
    std::string plotFilenamePrefix;
    std::string activationPatternDataFilename;
    std::ofstream activationPatternDataFile;
    std::string mapDataFilename;
    std::ofstream mapDataFile;
  };

  class TrajectoryPlotter;

  class ComparedGridMap : public ComparedMap {
  public:
    friend class TrajectoryPlotter;
    ComparedGridMap(Lab *, int index, GridMapParameters &);
    GridMap *getGridMap() { return gridMap; }
    void initializeGraphics();
    void display();
    void processAudio(float *buffer, unsigned long numFrames);
    void generatePlotFiles();
    void writePlotFilesContent();
    void startTrajectoryPlotting();
    void stopTrajectoryPlotting();
    void plotTrajectory();
  private:
    GridMapParameters parameters;
    GridMap *gridMap;
    TrajectoryPlotter *trajectoryPlotter;
    int spectrumResolution;
  };

  class ComparedCircleMap : public ComparedMap {
  public:
    ComparedCircleMap(Lab *, int index, CircleMapParameters &);
    void initializeGraphics();
    void display();
    void processAudio(float *buffer, unsigned long numFrames);
    void generatePlotFiles();
    void writePlotFilesContent();
  private:
    CircleMapParameters parameters;
    CircleMap *circleMap;
    CircleTopology *topology;
  };

  class TrajectoryPlotter {
  public:
    TrajectoryPlotter(ComparedGridMap *, Lab *);
    ~TrajectoryPlotter();
    void addDatum();
  private:
    ComparedGridMap *comparedMap;
    GridMap *map;
    std::string dataFilename;
    std::ofstream dataFile;
    typedef struct {
      float x;
      float y;
    } Point;
    std::vector<Point> points;

    void writePlotFilesContent();
  };

  void processCommandLineArguments();
  void usage();
  void initializeAudioProcessing();
  void initializeGraphics();
  void initializePlotting();
  void pretrain();
  void addGridMap();
  void addCircleMap();
  void addComparedMap(ComparedMap *map);
  void processAudioNonThreadSafe(float *);
  void generatePlotFiles();
  void toggleTrajectoryPlotting();

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
  bool plottingTrajectory;
  float pretrainSecs;
};
