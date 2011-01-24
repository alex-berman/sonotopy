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

#ifndef _SpectrumMap_hpp_
#define _SpectrumMap_hpp_

#include "Topology.hpp"
#include "SOM.hpp"
#include <vector>

namespace sonotopy {

class SpectrumMap {
public:
  typedef std::vector<float> ActivationPattern;

  SpectrumMap(Topology *, int spectrumResolution);
  ~SpectrumMap();
  void setNeighbourhoodParameter(float neighbourhoodParameter);
  void setLearningParameter(float learningParameter);
  void feedSpectrum(const float *, bool train = false);
  ActivationPattern* createActivationPattern() const;
  int getLastWinner() const;
  void getActivationPattern(ActivationPattern *) const;
  Topology* getTopology() const;

private:
  void createSom();
  void createSomInput();
  void createSomOutput();
  void spectrumToSomInput(const float *);
  void getMinAndMax(const std::vector<double> &values, float &min, float &max) const;

  SOM *som;
  Topology *topology;
  int spectrumResolution;
  SOM::Sample somInput;
  SOM::Output somOutput;
};

}

#endif
