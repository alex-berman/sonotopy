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

#include "SpectrumMapParameters.hpp"

using namespace sonotopy;

SpectrumMapParameters::SpectrumMapParameters() {
  trajectorySmoothness = 0.1f;
  adaptationStrategy = TimeBased;

  // parameters for time-based adaptation
  initialNeighbourhoodParameter = 1.0f;
  normalNeighbourhoodParameter = 0.1f;
  initialAdaptationTimeSecs = 0.05f;
  initialTrainingLengthSecs = 10.0;
  normalAdaptationTimeSecs = 3.0f;

  // parameters for error-driven adaptation
  adaptationPlasticity = 1.0f;
  neighbourhoodPlasticity = 1.0f;
  neighbourhoodParameterMin = 0.1f;
  errorThresholdLow = 0.0001f;
  errorThresholdHigh = 1.0f;
}
