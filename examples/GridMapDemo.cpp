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

#include "GridMapDemo.hpp"

using namespace std;
using namespace sonotopy;

GridMapDemo::GridMapDemo(int _argc, char **_argv) :
  Demo(_argc, _argv)
{
  initializeAudioProcessing();
}

void GridMapDemo::initializeAudioProcessing() {
  srand((unsigned) time(NULL));

  gridMap = new GridMap(audioParameters, gridMapParameters);
}

void GridMapDemo::initializeGraphics() {
  Demo::initializeGraphics();
  gridMapFrame = new SmoothGridMapFrame(gridMap);
}

void GridMapDemo::resizedWindow() {
  gridMapFrame->setSize(windowWidth, windowHeight);
  gridMapFrame->setPosition(0, 0);
}

void GridMapDemo::processDemoAudio(float *inputBuffer) {
  gridMap->feedAudio(inputBuffer, audioParameters.bufferSize);
}

void GridMapDemo::renderDemoGraphics() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  gridMapFrame->display();
}

int main(int argc, char **argv) {
  GridMapDemo(argc, argv).runDemo();
}
