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

#ifndef _SOM_hpp_
#define _SOM_hpp_

#include <vector>
#include "Topology.hpp"

namespace sonotopy {

class SOM {
public:
  typedef std::vector<double> Sample;
  typedef std::vector<double> Output;
  typedef unsigned int uint;

  SOM(uint inputSize, Topology *);
  ~SOM();
  Sample createSample(const double *) const;
  void setNeighbourhoodParameter(double); // 0-1
  void setLearningParameter(double); // 0-1
  void train(const Sample &input);
  uint getWinner(const Sample &input) const;
  uint getLastWinner() const;
  void getOutput(const Sample &input, Output &output) const;
  void getLastOutput(Output &output) const;
  double getOutputMin() const;
  double getOutputMax() const;
  void setModel(uint modelIndex, const Sample &);
  void setAllModels(const Sample &);
  void setRandomModelValues(double min = 0, double max = 1);

protected:
  class Model;

  typedef struct {
    Model *model;
    double strength; // from 0 to 1 where 1 is nearest neighbour
  } Neighbour;

  class Model {
  public:
    Model(const SOM *som, uint id);
    ~Model();
    void updateToInput(const Sample &input);
    void moveTowards(const std::vector<double > &sample, double amount);
    double getDistance(const Sample &input);
    void set(const Sample &);
    void setRandomValues(double min, double max);
    void updateNeighbourList();
  private:
    const SOM *parent;
    uint id;
    uint inputSize;
    double *values;
    std::vector<Neighbour> neighbours;
    double neighbourhoodParameter;
  };

  void createModels();
  void deleteModels();
  uint getWinnerAndStoreOutput(const Sample &input, Output &output);
  void updateNeighbourLists();

  uint inputSize;
  Topology *topology;
  uint numModels;
  double neighbourhoodParameter;
  double learningParameter;
  std::vector<Model *> models;
  double maxDistance; // max distance in euclidian space between two samples
  uint lastWinnerId;
  Output lastOutput;
  double outputMin, outputMax;
};

}

#endif
