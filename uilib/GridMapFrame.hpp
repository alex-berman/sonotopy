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

#include "Frame.hpp"
#include "ColorScheme.hpp"
#include <sonotopy/GridMap.hpp>

using namespace sonotopy;

class GridMapFrame : public Frame {
public:
  GridMapFrame(GridMap *, ColorScheme *);
  ~GridMapFrame();
  void render();
private:
  void renderActivationPattern();
  void renderCursor();
  GridMap *gridMap;
  ColorScheme *colorScheme;
  const SOM::ActivationPattern *activationPattern;
  int gridMapWidth;
  int gridMapHeight;
};

