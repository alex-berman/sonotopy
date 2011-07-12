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

#include "Lab.hpp"
#include <string.h>
#include <time.h>
#include <algorithm>
#include <string>
#include <math.h>

using namespace std;

Lab::Lab(int _argc, char **_argv) :
  GlWindow(_argc, _argv, 800, 600),
  AudioIO()
{
  argc = _argc;
  argv = _argv;

  processCommandLineArguments();
  initializeAudioProcessing();
  initializeAudio();
  initializeGraphics();
  initializePlotting();
  openAudioStream();
  glutMainLoop();
}

void Lab::processCommandLineArguments() {
  useAudioInputFile = false;
  echoAudio = false;
  plotError = false;
  mapCount = 0;
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
      else if(strcmp(argflag, "gridWidth") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.gridWidth = atoi(*argptr);
      }
      else if(strcmp(argflag, "gridHeight") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.gridHeight = atoi(*argptr);
      }
      else if(strcmp(argflag, "errorThresholdLow") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.errorThresholdLow =
	  circleMapParameters.errorThresholdLow =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "errorThresholdHigh") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.errorThresholdHigh =
	  circleMapParameters.errorThresholdHigh =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "errorIntegrationTimeMs") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.errorIntegrationTimeMs =
	  circleMapParameters.errorIntegrationTimeMs =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "adaptationPlasticity") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.adaptationPlasticity =
	  circleMapParameters.adaptationPlasticity =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "neighbourhoodPlasticity") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.neighbourhoodPlasticity =
	  circleMapParameters.neighbourhoodPlasticity =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "trajectorySmoothness") == 0) {
	argnr++;
	argptr++;
	gridMapParameters.trajectorySmoothness =
	  circleMapParameters.trajectorySmoothness =
	  atof(*argptr);
      }
      else if(strcmp(argflag, "gm") == 0) {
	addGridMap();
      }
      else if(strcmp(argflag, "cm") == 0) {
	addCircleMap();
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

  if(comparedMaps.size() == 0) {
    printf("No maps to compare\n");
    usage();
  }
}

void Lab::usage() {
  printf("Illegal usage\n");
  exit(0);
}

void Lab::addGridMap() {
  srand(t0);
  addComparedMap(new ComparedGridMap(this, mapCount, gridMapParameters));
}

void Lab::addCircleMap() {
  srand(t0);
  addComparedMap(new ComparedCircleMap(this, mapCount, circleMapParameters));
}

void Lab::addComparedMap(ComparedMap *map) {
  comparedMaps.push_back(map);
  mapCount++;
  circleMapParameters = CircleMapParameters();
  gridMapParameters = GridMapParameters();
}

void Lab::initializeAudioProcessing() {
  t0 = (unsigned) time(NULL);
  pthread_mutex_init(&mutex, NULL);
}

void Lab::initializeGraphics() {
  for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    (*i)->initializeGraphics();
}

void Lab::initializePlotting() {
  plotFileCount = 0;
  plottingTrajectory = false;
}

void Lab::processAudio(float *inputBuffer) {
  pthread_mutex_lock(&mutex);
  for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    (*i)->processAudio(inputBuffer, audioParameters.bufferSize);
  pthread_mutex_unlock(&mutex);
}

void Lab::display() {
  if(pthread_mutex_trylock(&mutex) != 0)
    return;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++) {
    (*i)->display();
    if(plottingTrajectory)
      (*i)->plotTrajectory();
  }

  pthread_mutex_unlock(&mutex);

  glutSwapBuffers();
}

void Lab::resizedWindow() {
  int margin = (int) (0.01 * windowWidth);
  int frameSize = (windowWidth- margin * (comparedMaps.size() - 1)) / comparedMaps.size();
  frameSize = min(frameSize, windowHeight);
  int offsetTop = (windowHeight - frameSize) / 2;
  int px = (windowWidth - frameSize * comparedMaps.size() - margin * (comparedMaps.size()-1)) / 2;
  Frame *frame;
  for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++) {
    frame = (*i)->getFrame();
    frame->setSize(frameSize, frameSize);
    frame->setPosition(px, offsetTop);
    px += frameSize + margin;
  }
}

void Lab::glKeyboard(unsigned char key, int x, int y) {
  switch(key) {
  case 'p':
    generatePlotFiles();
    break;

  case 't':
    toggleTrajectoryPlotting();
    break;
  }
}

void Lab::generatePlotFiles() {
  pthread_mutex_lock(&mutex);
  for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    (*i)->generatePlotFile();
  pthread_mutex_unlock(&mutex);
  plotFileCount++;
}

void Lab::toggleTrajectoryPlotting() {
  if(plottingTrajectory) {
    for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
      (*i)->stopTrajectoryPlotting();
    plottingTrajectory = false;
  }
  else {
    for(vector<ComparedMap*>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
      (*i)->startTrajectoryPlotting();
    plottingTrajectory = true;
  }
}


Lab::ErrorGraph::ErrorGraph(Lab *_parent, const SpectrumMap *_map) {
  parent = _parent;
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

Lab::ErrorGraph::~ErrorGraph() {
  delete [] buffer;
  delete circularBufferMin;
  delete circularBufferMax;
}

void Lab::ErrorGraph::update() {
  circularBufferMin->write(smootherMin.smooth(map->getErrorMin()));
  circularBufferMin->moveReadHead(1);
  circularBufferMax->write(smootherMax.smooth(map->getErrorMax()));
  circularBufferMax->moveReadHead(1);
}

void Lab::ErrorGraph::render(Frame *frame) {
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

  char text[256];
  glColor3f(0, 1, 0);
  sprintf(text, "min: %.5f  max: %.5f  graph max: %.5f",
	  map->getErrorMin(), map->getErrorMax(), maxValue);
  parent->glText(frame->getLeft(), frame->getBottom() + 20, text);

  sprintf(text, "adaptTime: %.5f neighParam: %.5f",
	  map->getAdaptationTimeSecs(),
	  map->getNeighbourhoodParameter());
  parent->glText(frame->getLeft(), frame->getBottom() + 40, text);
}


Lab::ComparedMap::ComparedMap(Lab *_parent, int _index) {
  index = _index;
  parent = _parent;
}

void Lab::ComparedMap::generatePlotFile() {
  ostringstream activationPatternDataFilenameSS, scriptFilenameSS;
  ostringstream plotFilenamePrefixSS;
  plotFilenamePrefixSS << "plots/plot" << index << "_" << parent->plotFileCount;
  plotFilenamePrefix = plotFilenamePrefixSS.str();
  activationPatternDataFilenameSS << plotFilenamePrefix << "_ap.dat";
  scriptFilenameSS << plotFilenamePrefix << ".plot";
  activationPatternDataFilename = activationPatternDataFilenameSS.str();
  activationPatternDataFile.open(activationPatternDataFilename.c_str());
  scriptFilename = scriptFilenameSS.str();
  scriptFile.open(scriptFilename.c_str());

  writePlotFilesContent();

  scriptFile.close();
  activationPatternDataFile.close();
}


Lab::ComparedGridMap::ComparedGridMap(Lab *_parent, int _index, GridMapParameters &_parameters)
  : ComparedMap(_parent, _index) {
  parameters = _parameters;
  gridMap = new GridMap(parent->audioParameters, parameters);
  spectrumResolution = gridMap->getSpectrumResolution();
}

void Lab::ComparedGridMap::initializeGraphics() {
  frame = new SmoothGridMapFrame(gridMap);
  if(parent->plotError)
    errorGraph = new ErrorGraph(parent, gridMap);
}

void Lab::ComparedGridMap::processAudio(float *inputBuffer, unsigned long numFrames) {
  gridMap->feedAudio(inputBuffer, numFrames);
  if(parent->plotError) {
    errorGraph->update();
  }
}

void Lab::ComparedGridMap::writePlotFilesContent() {
  writeActivationPatternData();
  writeMapData();
  writeScriptFile();
}

void Lab::ComparedGridMap::writeScriptFile() {
  float rangeX1 = -0.5;
  float rangeX2 = parameters.gridWidth - 0.5;
  float rangeY1 = -0.5;
  float rangeY2 = parameters.gridHeight - 0.5;

  scriptFile << "set multiplot" << endl;

  scriptFile << "set border 0" << endl
	     << "unset xtics; unset ytics; unset ztics" << endl
	     << "unset colorbox" << endl;

  scriptFile << "splot [" << rangeX1 << ":" << rangeX2 << "] "
	     << "[" << rangeY1 << ":" << rangeY2 << "] [0:1] \\" << endl;
  for(int y = 0; y < parameters.gridHeight; y++) {
    for(int x = 0; x < parameters.gridWidth; x++) {
      if(y != 0 || x != 0)
	scriptFile << ", \\" << endl;
      scriptFile << "  '" << getMapDataFilename(x, y) << "' with pm3d title ''";
    }
  }
  scriptFile << endl;

  scriptFile << "set border 0" << endl
	     << "unset xtics; unset ytics; unset ztics" << endl
	     << "unset pm3d" << endl;
  scriptFile << "splot [" << rangeX1 << ":" << rangeX2 << "] "
	     << "[" << rangeY1 << ":" << rangeY2 << "] [0:1] \\" << endl
	     << "  '" << activationPatternDataFilename << "'"
	     << " with lines lc rgb 'black' title ''" << endl;

  scriptFile << "unset multiplot" << endl;
}

void Lab::ComparedGridMap::writeActivationPatternData() {
  const SOM::ActivationPattern *activationPattern = gridMap->getActivationPattern();
  SOM::ActivationPattern::const_iterator activationPatternIterator =
    activationPattern->begin();
  float v;
  for(int y = 0; y < parameters.gridHeight; y++) {
    for(int x = 0; x < parameters.gridWidth; x++) {
      v = *activationPatternIterator++;
      activationPatternDataFile << x << " " << y << " " << v << endl;
    }
    activationPatternDataFile << endl;
  }
}

void Lab::ComparedGridMap::writeMapData() {
  for(int y = 0; y < parameters.gridHeight; y++) {
    for(int x = 0; x < parameters.gridWidth; x++) {
      generateMapDataFile(x, y);
    }
  }
}

string Lab::ComparedGridMap::getMapDataFilename(int gridX, int gridY) {
  ostringstream mapDataFilenameSS;
  std::string mapDataFilename;
  mapDataFilenameSS << plotFilenamePrefix << "_map"
		    << gridX << "_" << gridY << ".dat";
  return mapDataFilenameSS.str();
}

void Lab::ComparedGridMap::generateMapDataFile(int gridX, int gridY) {
  bool applyLog = (gridMap->getSpectrumAnalyzer()->getPowerScale() == SpectrumAnalyzer::Amplitude);
  string mapDataFilename = getMapDataFilename(gridX, gridY);
  std::ofstream mapDataFile(mapDataFilename.c_str());

  const float *spectrum = gridMap->getModel(gridX, gridY);
  const static float z = 0;
  const static float margin = 0.1;
  float x1 = -0.5 + gridX + margin;
  float x2 = -0.5 + gridX + 1 - margin;
  float y1 = -0.5 + gridY + margin;
  float y2 = -0.5 + gridY + 1 - margin;
  float color, y;
  for(int i = 0; i < spectrumResolution; i++) {
    color = spectrum[i];
    if(applyLog)
      color = log(color) + 1;
    y = y1 + (y2 - y1) * (float) i / (spectrumResolution-1);
    mapDataFile << x1 << " " << y << " " << z << " " << color << endl;
    mapDataFile << x2 << " " << y << " " << z << " " << color << endl;
    mapDataFile << endl;
  }

  mapDataFile.close();
}

void Lab::ComparedGridMap::startTrajectoryPlotting() {
  trajectoryPlotter = new TrajectoryPlotter(this);
}

void Lab::ComparedGridMap::stopTrajectoryPlotting() {
  delete trajectoryPlotter;
}

void Lab::ComparedGridMap::plotTrajectory() {
  trajectoryPlotter->addDatum();
}

void Lab::ComparedGridMap::display() {
  frame->display();
  if(parent->plotError)
    errorGraph->render(frame);
}



Lab::ComparedCircleMap::ComparedCircleMap(Lab *_parent, int _index, CircleMapParameters &_parameters)
  : ComparedMap(_parent, _index) {
  parameters = _parameters;
  circleMap = new CircleMap(parent->audioParameters, parameters);
  topology = (CircleTopology*) circleMap->getTopology();
}

void Lab::ComparedCircleMap::initializeGraphics() {
  frame = new SmoothCircleMapFrame(circleMap);
  if(parent->plotError)
    errorGraph = new ErrorGraph(parent, circleMap);
}

void Lab::ComparedCircleMap::processAudio(float *inputBuffer, unsigned long numFrames) {
  circleMap->feedAudio(inputBuffer, numFrames);
  if(parent->plotError) {
    errorGraph->update();
  }
}

void Lab::ComparedCircleMap::writePlotFilesContent() {
  float angle = circleMap->getAngle();
  scriptFile << "set arrow 1 from 0,0,0 to " <<
    cos(angle) << "," << sin(angle) << ",0 linewidth 2" << endl;
  scriptFile << "splot '" << activationPatternDataFilename << "' with lines title ''" << endl;

  const SOM::ActivationPattern *activationPattern = circleMap->getActivationPattern();
  CircleTopology::Node node;
  float z;
  const static float r = 0.7;
  for(unsigned int i = 0; i <= topology->getNumNodes(); i++) {
    int n = i % topology->getNumNodes();
    node = topology->getNode(n);
    z = 1 - (*activationPattern)[n];
    activationPatternDataFile <<   cos(node.angle) << " " <<   sin(node.angle) << " " << z << endl;
    activationPatternDataFile << r*cos(node.angle) << " " << r*sin(node.angle) << " " << z << endl;
    activationPatternDataFile << endl;
  }
}

void Lab::ComparedCircleMap::display() {
  frame->display();
  if(parent->plotError)
    errorGraph->render(frame);
}


Lab::TrajectoryPlotter::TrajectoryPlotter(ComparedGridMap *comparedMap) {
  map = comparedMap->getGridMap();
  int index = comparedMap->getIndex();
  ostringstream dataFilenameSS, scriptFilenameSS;
  dataFilenameSS << "tplot" << index << ".dat";
  scriptFilenameSS << "tplot" << index << ".plot";
  dataFilename = dataFilenameSS.str();
  scriptFilename = scriptFilenameSS.str();
  dataFile.open(dataFilename.c_str());
  scriptFile.open(scriptFilename.c_str());
}

Lab::TrajectoryPlotter::~TrajectoryPlotter() {
  writePlotFilesContent();
  dataFile.close();
  scriptFile.close();
}

void Lab::TrajectoryPlotter::writePlotFilesContent() {
  scriptFile << "set palette rgbformulae -2,3,3" << endl;
  scriptFile << "unset colorbox" << endl;
  scriptFile << "splot '" << dataFilename << "' with lines lc palette z title ''" << endl;

  const static float z = 0;
  float color;
  int numPoints = points.size();
  int i = 0;
  for(vector<Point>::iterator p = points.begin(); p != points.end(); p++) {
    color = (float) (i + 1) / numPoints;
    dataFile << p->x << " " << p->y << " " << z << " " << color << endl;
    i++;
  }
}

void Lab::TrajectoryPlotter::addDatum() {
  Point point;
  map->getCursor(point.x, point.y);
  points.push_back(point);
}

int main(int argc, char **argv) {
  Lab(argc, argv);
}
