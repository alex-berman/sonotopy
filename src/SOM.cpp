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

#include "SOM.hpp"
#include <stdlib.h>
#include <math.h>
#include <cassert>

using namespace sonotopy;
using namespace std;

SOM::SOM(uint _inputSize, Topology *_topology) {
  assert(_inputSize != 0);
  inputSize = _inputSize;
  topology = _topology;
  numModels = topology->getNumNodes();
  setLearningParameter(0.5);
  outputMin = 0;
  outputMax = 0;
  maxDistance = ::sqrt((float)inputSize); // sqrt(1² + 1² ... inputSize times)
  createModels();
}

SOM::~SOM() {
  deleteModels();
}

void SOM::createModels() {
  for(uint id = 0; id < numModels; id++)
    models.push_back(new Model(this, id));
}

void SOM::deleteModels() {
  for(vector<Model*>::iterator i = models.begin(); i != models.end(); ++i)
    delete *i;
}

SOM::Sample SOM::createSample(const double *values) const {
  Sample sample(inputSize);
  const double *valuePtr = values;
  Sample::iterator samplePtr = sample.begin();
  for(uint k = 0; k < inputSize; k++)
    *samplePtr++ = *valuePtr++;
  return sample;
}

void SOM::setNeighbourhoodParameter(double _neighbourhoodParameter) {
  neighbourhoodParameter = _neighbourhoodParameter;
  topology->setVicinityFactor((float)neighbourhoodParameter);
}

void SOM::setLearningParameter(double x) {
  learningParameter = x;
}

SOM::uint SOM::getWinner(const Sample &input) const {
  double diff;
  double closest = 0;
  uint winner = 0;
  uint modelIndex = 0;
  Model *model;
  for(vector<Model*>::const_iterator i = models.begin(); i != models.end(); ++i) {
    model = *i;
    diff = model->getDistance(input);
    if(i == models.begin() || diff < closest) {
      closest = diff;
      winner = modelIndex;
    }
    modelIndex++;
  }
  return winner;
}

SOM::uint SOM::getLastWinner() const {
  return lastWinnerId;
}

void SOM::train(const Sample &input) {
  lastWinnerId = getWinnerAndStoreOutput(input, lastOutput);
  Model *winnerModel = models[lastWinnerId];
  winnerModel->updateToInput(input);
}

SOM::uint SOM::getWinnerAndStoreOutput(const Sample &input, Output &output) {
  uint winnerId = 0;
  double distance, localDistanceMin = 0, localDistanceMax = 0;
  double modelOutput, localOutputMin = 0, localOutputMax = 0;
  Model *model;

  output.clear();
  uint modelId = 0;
  for(vector<Model*>::iterator i = models.begin(); i != models.end(); ++i) {
    model = *i;
    distance = model->getDistance(input);
    modelOutput = (float) (::sqrt(distance) / maxDistance);
    if(i == models.begin()) {
      localDistanceMin = localDistanceMax = distance;
      winnerId = modelId;
      localOutputMin = localOutputMax = modelOutput;
    }
    else if(distance < localDistanceMin) {
      localDistanceMin = distance;
      winnerId = modelId;
      localOutputMin = modelOutput;
    }
    else if(distance > localDistanceMax) {
      localDistanceMax = distance;
      localOutputMax = modelOutput;
    }
    output.push_back(modelOutput);
    modelId++;
  }

  outputMin = localOutputMin;
  outputMax = localOutputMax;
  return winnerId;
}

void SOM::getOutput(const Sample &input, Output &output) const {
  ((SOM *)this)->getWinnerAndStoreOutput(input, output);
}

void SOM::getLastOutput(Output &output) const {
  output = lastOutput;
}

double SOM::getOutputMin() const {
  return outputMin;
}

double SOM::getOutputMax() const {
  return outputMax;
}

void SOM::setModel(uint modelIndex, const Sample &sample) {
  models[modelIndex]->set(sample);
}

void SOM::setAllModels(const Sample &sample) {
  Model *model;
  for(vector<Model*>::iterator i = models.begin(); i != models.end(); ++i) {
    model = *i;
    model->set(sample);
  }
}

void SOM::setRandomModelValues(double min, double max) {
  Model *model;
  for(vector<Model*>::iterator i = models.begin(); i != models.end(); ++i) {
    model = *i;
    model->setRandomValues(min, max);
  }
}

void SOM::updateNeighbourLists() {
  for(vector<Model*>::iterator i = models.begin(); i != models.end(); ++i)
    (*i)->updateNeighbourList();
}

SOM::Model::Model(const SOM *_parent, uint _id) {
  id = _id;
  parent = _parent;
  inputSize = parent->inputSize;
  values = new double [inputSize];
  neighbourhoodParameter = 0;
}

SOM::Model::~Model() {
  delete [] values;
}

void SOM::Model::updateToInput(const SOM::Sample &input) {
  double learningParameter = parent->learningParameter;
  moveTowards(input, learningParameter);

  updateNeighbourList();
  for(std::vector<Neighbour>::iterator i = neighbours.begin(); i != neighbours.end(); i++)
    i->model->moveTowards(input, learningParameter * i->strength);
}

void SOM::Model::moveTowards(const std::vector<double > &sample, double amount) {
  double *valuePtr = values;
  Sample::const_iterator samplePtr = sample.begin();
  for(uint k = 0; k < inputSize; k++) {
    *valuePtr += amount * (*samplePtr - *valuePtr);
    valuePtr++;
    samplePtr++;
  }
}

double SOM::Model::getDistance(const Sample &input) {
  double *valuePtr = values;
  Sample::const_iterator samplePtr = input.begin();
  double d;
  double distance = 0;
  for(uint k = 0; k < inputSize; k++) {
    d = *valuePtr++ - *samplePtr++;
    distance += d * d;
  }
  return distance;
}

void SOM::Model::set(const Sample &sample) {
  double *valuePtr = values;
  Sample::const_iterator samplePtr = sample.begin();
  for(uint k = 0; k < inputSize; k++)
    *valuePtr++ = *samplePtr++;
}

void SOM::Model::setRandomValues(double min, double max) {
  double *valuePtr = values;
  for(uint k = 0; k < inputSize; k++)
    *valuePtr++ = min + (max - min) * (double) rand() / RAND_MAX;
}

void SOM::Model::updateNeighbourList() {
  if(neighbourhoodParameter != parent->neighbourhoodParameter) {
    std::vector<Topology::Neighbour> topologyNeighbours;
    parent->topology->getNeighbours(id, topologyNeighbours);
    neighbours.clear();
    Neighbour neighbour;
    for(std::vector<Topology::Neighbour>::iterator i = topologyNeighbours.begin(); i != topologyNeighbours.end(); i++) {
      neighbour.model = parent->models[i->nodeId];
      neighbour.strength = i->strength;
      neighbours.push_back(neighbour);
    }
    neighbourhoodParameter = parent->neighbourhoodParameter;
  }
}
