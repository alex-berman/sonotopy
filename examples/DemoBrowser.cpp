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

#include "DemoBrowser.hpp"

using namespace std;
using namespace sonotopy;

DemoBrowser::DemoBrowser(int _argc, char **_argv) :
  Demo(_argc, _argv)
{
  SPACING = 5;
  SINGLE_FRAME_RELATIVE_SIZE = 0.8;
  initializeAudioProcessing();
}

void DemoBrowser::initializeAudioProcessing() {
  srand((unsigned) time(NULL));

  gridMap = new GridMap(audioParameters, spectrumAnalyzerParameters, gridMapParameters);
  spectrumAnalyzer = gridMap->getSpectrumAnalyzer();
  spectrumBinDivider = gridMap->getSpectrumBinDivider();
  circleMap = new CircleMap(audioParameters, spectrumAnalyzerParameters, circleMapParameters);
  beatTracker = new BeatTracker(spectrumBinDivider->getNumBins(), audioParameters.bufferSize, audioParameters.sampleRate);
  eventDetector = new EventDetectionPrinter(audioParameters);
  createDisjointGridMap();
}

void DemoBrowser::createDisjointGridMap() {
  // create disjoint map consisting of two sections: upper-left and bottom-right corner
  int w = disjointGridMapParameters.gridWidth * 0.8;
  int h = disjointGridMapParameters.gridHeight * 0.8;
  vector<DisjointGridTopology::Node> nodes;
  for(int y = 0; y < h; y++) {
    int rw = w * y / h;
    for(int x = 0; x < rw; x++) {
      nodes.push_back(DisjointGridTopology::Node(disjointGridMapParameters.gridWidth-x-1, h-y));
      nodes.push_back(DisjointGridTopology::Node(x, disjointGridMapParameters.gridHeight-(h-y)-1));
    }
  }
  disjointGridMap = new DisjointGridMap(audioParameters,
					spectrumAnalyzerParameters,
					disjointGridMapParameters,
					nodes);
}

void DemoBrowser::initializeGraphics() {
  printf("press right or left arrow on keyboard to switch between visualizations\n");
  Demo::initializeGraphics();

  normalizeSpectrum = (spectrumAnalyzer->getPowerScale() == Amplitude);
  waveformFrame = new WaveformFrame(monauralInputBuffer, audioParameters.bufferSize);
  spectrumFrame = new SpectrumFrame(spectrumAnalyzer, normalizeSpectrum);
  spectrumBinsFrame = new SpectrumBinsFrame(spectrumBinDivider, normalizeSpectrum);
  gridMapFrame = new GridMapFrame(gridMap, colorScheme);
  disjointGridMapFrame = new GridMapFrame(disjointGridMap, colorScheme);
  enlargedGridMapFrame = new SmoothGridMapFrame(gridMap, colorScheme);
  gridMapTrajectoryFrame = new GridMapTrajectoryFrame(gridMap);
  beatTrackerFrame = new BeatTrackerFrame(beatTracker);
  isolinesFrame = new IsolinesFrame(gridMap);
  circleMapFrame = new CircleMapFrame(circleMap, beatTracker);
  enlargedCircleMapFrame = new SmoothCircleMapFrame(circleMap);

  for(int i = 0; i < 20; i++)
    dancers.push_back(Dancer(circleMap, beatTracker, this));

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);

  moveToScene(Scene_Mixed);
}

void DemoBrowser::moveToScene(int _sceneNum) {
  sceneNum = _sceneNum;
  resizeFrames();
}

void DemoBrowser::glSpecial(int key, int x, int y) {
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

void DemoBrowser::resizedWindow() {
  resizeFrames();
}

void DemoBrowser::resizeFrames() {
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

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->setSize(displayWidth, rowHeight);
      waveformFrame->setPosition(SPACING, y0);

      spectrumFrame->setSize(displayWidth, rowHeight);
      spectrumFrame->setPosition(SPACING, y1);

      spectrumBinsFrame->setSize(displayWidth, rowHeight);
      spectrumBinsFrame->setPosition(SPACING, y2);

      beatTrackerFrame->setSize(columnWidth / 2, rowHeight);
      beatTrackerFrame->setPosition(SPACING, y3);

      circleMapFrame->setSize(columnWidth, rowHeight);
      circleMapFrame->setPosition(SPACING + displayWidth - columnWidth*3 - SPACING*2, y3);

      gridMapFrame->setSize(columnWidth, rowHeight);
      gridMapFrame->setPosition(SPACING + displayWidth - columnWidth*2 - SPACING, y3);

      disjointGridMapFrame->setSize(columnWidth, rowHeight);
      disjointGridMapFrame->setPosition(SPACING + displayWidth - columnWidth, y3);
      break;

    case Scene_EnlargedCircleMap:
      enlargedCircleMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedCircleMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_EnlargedGridMap:
      enlargedGridMapFrame->setSize(singleFrameSize, singleFrameSize);
      enlargedGridMapFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_GridMapTrajectory:
      gridMapTrajectoryFrame->setSize(singleFrameSize, singleFrameSize);
      gridMapTrajectoryFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;

    case Scene_Isolines:
      isolinesFrame->setSize(singleFrameSize, singleFrameSize);
      isolinesFrame->setPosition(singleFrameOffsetLeft, singleFrameOffsetTop);
      break;
  }
}

void DemoBrowser::processDemoAudio(float *inputBuffer) {
  gridMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  disjointGridMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  circleMap->feedAudio(inputBuffer, audioParameters.bufferSize);
  beatTracker->feedFeatureVector(spectrumBinDivider->getBinValues());
  eventDetector->feedAudio(inputBuffer, audioParameters.bufferSize);
}

void DemoBrowser::renderDemoGraphics() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  switch(sceneNum) {
    case Scene_Mixed:
      waveformFrame->display();
      spectrumFrame->display();
      spectrumBinsFrame->display();
      gridMapFrame->display();
      disjointGridMapFrame->display();
      circleMapFrame->display();
      beatTrackerFrame->display();
      break;

    case Scene_Dancers:
      updateDancers();
      renderDancers();
      break;

    case Scene_EnlargedCircleMap:
      enlargedCircleMapFrame->display();
      break;

    case Scene_EnlargedGridMap:
      enlargedGridMapFrame->display();
      break;

    case Scene_GridMapTrajectory:
      gridMapTrajectoryFrame->display();
      break;

    case Scene_Isolines:
      isolinesFrame->display();
      break;
  }
}

void DemoBrowser::updateDancers() {
  float timeIncrementSecs = timeIncrement / 1000;
  for(vector<Dancer>::iterator dancer = dancers.begin(); dancer != dancers.end(); dancer++)
    dancer->update(timeIncrementSecs);
}

void DemoBrowser::renderDancers() {
  for(vector<Dancer>::iterator dancer = dancers.begin(); dancer != dancers.end(); dancer++)
    dancer->render();
}

int main(int argc, char **argv) {
  DemoBrowser(argc, argv).runDemo();
}
