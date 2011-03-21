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
  openAudioStream();
  glutMainLoop();
}

void Lab::processCommandLineArguments() {
  useAudioInputFile = false;
  echoAudio = false;
  plotError = false;
  showAdaptationValues = false;
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
	  gridMapParameters.adaptationStrategy =
	    SpectrumMapParameters::TimeBased;
	}
	else if(strcmp(*argptr, "error") == 0) {
	  gridMapParameters.adaptationStrategy =
	    SpectrumMapParameters::ErrorDriven;
	}
	else {
	  printf("Unknown adaptation strategy %s\n", *argptr);
	  usage();
	}
      }
      else if(strcmp(argflag, "gm") == 0) {
	addGridMap();
      }
      else if(strcmp(argflag, "showadapt") == 0) {
	showAdaptationValues = true;
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
  comparedMaps.push_back(ComparedMap(this, gridMapParameters));
  gridMapParameters = GridMapParameters();
}

void Lab::initializeAudioProcessing() {
  srand((unsigned) time(NULL));
}

void Lab::initializeGraphics() {
  for(vector<ComparedMap>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    i->initializeGraphics();
}

void Lab::processAudio(float *inputBuffer) {
  for(vector<ComparedMap>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    i->processAudio(inputBuffer, audioParameters.bufferSize);
}

void Lab::display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  for(vector<ComparedMap>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++)
    i->display();

  glutSwapBuffers();

  /*
  if(showAdaptationValues) {
    if(frameCount % 50 == 0) {      
      printf("errorLevel=%.5f adaptationTimeSecs=%.5f neighbourhoodParameter=%.5f\n",
	     gridMap->getErrorLevel(),
	     gridMap->getAdaptationTimeSecs(),
	     gridMap->getNeighbourhoodParameter());
    }
  }
  */
}

void Lab::resizedWindow() {
  int margin = (int) (0.01 * windowWidth);
  int frameSize = (int) windowWidth / comparedMaps.size()
    - margin * (comparedMaps.size() - 1);
  frameSize = min(frameSize, windowHeight);
  int offsetTop = (windowHeight - frameSize) / 2;
  int px = (windowWidth - frameSize * comparedMaps.size() - margin * (comparedMaps.size()-1)) / 2;
  for(vector<ComparedMap>::iterator i = comparedMaps.begin(); i != comparedMaps.end(); i++) {
    i->getFrame()->setSize(frameSize, frameSize);
    i->getFrame()->setPosition(px, offsetTop);
    px += frameSize + margin;
  }
}

Lab::ErrorPlotter::ErrorPlotter(const Lab *parent, const SpectrumMap *_map) {
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

Lab::ErrorPlotter::~ErrorPlotter() {
  delete [] buffer;
  delete circularBufferMin;
  delete circularBufferMax;
}

void Lab::ErrorPlotter::update() {
  circularBufferMin->write(smootherMin.smooth(map->getErrorMin()));
  circularBufferMin->moveReadHead(1);
  circularBufferMax->write(smootherMax.smooth(map->getErrorMax()));
  circularBufferMax->moveReadHead(1);
}

void Lab::ErrorPlotter::render(Frame *frame) {
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
}

Lab::ComparedMap::ComparedMap(const Lab *_parent, GridMapParameters &_parameters) {
  parent = _parent;
  parameters = _parameters;
  gridMap = new GridMap(parent->audioParameters, parameters);
}

void Lab::ComparedMap::initializeGraphics() {
  gridMapFrame = new SmoothGridMapFrame(gridMap);
  if(parent->plotError)
    errorPlotter = new ErrorPlotter(parent, gridMap);
}

void Lab::ComparedMap::processAudio(float *inputBuffer, unsigned long numFrames) {
  gridMap->feedAudio(inputBuffer, numFrames);
  if(parent->plotError) {
    errorPlotter->update();
  }
}

void Lab::ComparedMap::display() {
  gridMapFrame->display();
  if(parent->plotError)
    errorPlotter->render(gridMapFrame);
}

int main(int argc, char **argv) {
  Lab(argc, argv);
}
