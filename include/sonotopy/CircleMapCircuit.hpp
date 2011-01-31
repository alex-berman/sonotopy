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

#ifndef CIRCLEMAPCIRCUIT_HPP
#define CIRCLEMAPCIRCUIT_HPP

#include "SpectrumMapCircuit.hpp"
#include "AudioParameters.hpp"
#include "CircleMapCircuitParameters.hpp"

namespace sonotopy {

class CircleMapCircuit : public SpectrumMapCircuit
{
public:
  CircleMapCircuit(const AudioParameters &, const CircleMapCircuitParameters &);
  float getAngle();
};

}

#endif // CIRCLEMAPCIRCUIT_HPP

