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

#ifndef _IsolineExtractor_hpp_
#define _IsolineExtractor_hpp_

#include <vector>
#include <sonotopy/TwoDimArray.hpp>

using namespace sonotopy;

class IsolineExtractor {
public:
  class Pixel {
  public:
    bool operator==(const Pixel &v) {
      if(v.x != x) return false;
      if(v.y != y) return false;
      return true;
    }
    bool operator!=(const Pixel &v) {
      if(v.x != x) return true;
      if(v.y != y) return true;
      return false;
    }
    int x;
    int y;
  };

	typedef struct {
		float x;
		float y;
	} Point;

	typedef struct {
		std::vector<Pixel> pixels;
		std::vector<Point> points;
    bool enclosed;
	} Curve;

  typedef std::vector<Curve> CurveSet;

	typedef struct {
		int x;
		int y;
		unsigned char *traceImagePtr;
		float *inputMapPtr;
	} Neighbour;

  IsolineExtractor(int _gridWidth, int _gridHeight);
  void setThreshold(float);
  void setMap(const TwoDimArray<float> &map);
  void process();
  void interpolate();
  void smooth(float amount);
  unsigned char *getIsolinesMap();
  unsigned char *getThresholdImage();
  CurveSet *getCurves();
  int getGridWidth() { return w; }
  int getGridHeight() { return h; }

private:
	float *getInputMapPtr(int x, int y);
  float getInputValue(int x, int y);
  unsigned char getThresholdValue(int x, int y);
  unsigned char getTraceImageValue(int x, int y);
  unsigned char *traceImagePtr(int x, int y);
  unsigned char *output(int x, int y);
  bool thresholdEdge(int x, int y);
  bool findLineStart(Pixel &);
  bool findNeighbour(const Pixel &v1, Pixel &v2);
  bool isLineStart(int x, int y);
	void buildNeighbourhoods();
  std::vector<Neighbour> *getNeighbourhood(int x, int y);
	void interpolateCurve(Curve &);
	void smoothCurve(Curve &, float amount);
	void interpolatePoint(const Pixel &pixel, Point &point);

  int w;
  int h;
	float threshold;
  float *inputMap;
  unsigned char *outputMap;
  unsigned char *thresholdImage;
  unsigned char *traceImage;
  unsigned char *traceImageTemp;
  int imageSize;
  CurveSet curves;
	std::vector<Neighbour> *neighbourhoods;
};

#endif

