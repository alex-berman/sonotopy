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

#include "IsolineExtractor.hpp"
#include <map>
#include <string.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

IsolineExtractor::IsolineExtractor(int _gridWidth, int _gridHeight) {
  w = _gridWidth;
  h = _gridHeight;

  inputMap       = new float [w * h];
  outputMap      = new unsigned char [w * h];
  thresholdImage = new unsigned char [w * h];
  traceImage     = new unsigned char [w * h];
  traceImageTemp = new unsigned char [w * h];
  imageSize = w * h;

	buildNeighbourhoods();

	setThreshold(0.5);
}

void IsolineExtractor::setThreshold(float t) { threshold = t; }

unsigned char *IsolineExtractor::getIsolinesMap() { return outputMap; }
unsigned char *IsolineExtractor::getThresholdImage() { return thresholdImage; }
IsolineExtractor::CurveSet *IsolineExtractor::getCurves() { return &curves; }

float *IsolineExtractor::getInputMapPtr(int x, int y) {
  return inputMap + y * w + x;
}

float IsolineExtractor::getInputValue(int x, int y) {
  return *(getInputMapPtr(x, y));
}

void IsolineExtractor::setMap(const TwoDimArray<float> &inputMapArray) {
  float *inputP = inputMap;
  TwoDimArray<float>::Iterator inputMapArrayIterator = inputMapArray.begin();
  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      *inputP++ = *inputMapArrayIterator->value;
      inputMapArrayIterator++;
    }
  }
}

void IsolineExtractor::process() {
  // thresholding
  float *inputP = inputMap;
  unsigned char *thresholdImageP = thresholdImage;
  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      *thresholdImageP++ = (*inputP++) > threshold ? 1 : 0;
    }
  }


  // edge detection
  memset(outputMap, 0, imageSize);
  unsigned char *outputP;
  for(int y = 0; y < (h); y++) {
    outputP = outputMap + y * w;
    for(int x = 0; x < (w); x++) {
      if(thresholdEdge(x, y))
        *outputP = 1;
      outputP++;
    }
  }

  // clean up edge image
  for(int y = 0; y < (h-1); y++) {
    for(int x = 0; x < (w-1); x++) {
      if(*output(x, y)) {
        if(*output(x+1, y+1)) {
          *(output(x+1, y)) = 0;
          *(output(x, y+1)) = 0;
          continue;
        }
      }
      if(*output(x+1, y)) {
        if(*output(x, y+1)) {
          *(output(x, y)) = 0;
          *(output(x+1, y+1)) = 0;
          continue;
        }
      }
    }
  }

  // trace lines
  memcpy(traceImage, outputMap, imageSize);
  Pixel v0, v1, v2;
  Curve curve;
	vector<Pixel> ps;
  curves.clear();
  while(true) {
    // find curve
    bool foundCurve = false;
    for(int y = 0; y < h; y++) {
      for(int x = 0; x < w; x++) {
        if(getTraceImageValue(x, y)) {
          foundCurve = true;
          v0.x = x;
          v0.y = y;
          break;
        }
      }
    }
    if(!foundCurve) break;

    // trace curve until end point (curve segment) or returning to beginning (enclosed curve)
    memcpy(traceImageTemp, traceImage, imageSize);
    v1 = v2 = v0;
    *traceImagePtr(v0.x, v0.y) = 0;
    while(findNeighbour(v1, v2))
      v1 = v2;
    v0 = v2;

    // trace neighbours iteratively
    memcpy(traceImage, traceImageTemp, imageSize);
    v1 = v0;
    ps.clear();
    ps.push_back(v1);
    *traceImagePtr(v0.x, v0.y) = 0;
    while(findNeighbour(v1, v2)) {
      ps.push_back(v2);
      v1 = v2;
    }
    if(v2.x != v1.x || v2.y != v1.y)
      ps.push_back(v2);
    curve.enclosed = false;
    if(abs(v1.x - v0.x) <= 1 && abs(v1.y - v0.y) <= 1) {
      // enclosed (last point neighbours first point)
      ps.push_back(v0);
      curve.enclosed = true;
    }
		curve.pixels = ps;
    curves.push_back(curve);
  }
}

bool IsolineExtractor::findLineStart(Pixel &v) {
  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      if(isLineStart(x, y)) {
        *(traceImagePtr(x, y)) = 0;
        v.x = x;
        v.y = y;
        return true;
      }
    }
  }
  return false;
}

bool IsolineExtractor::isLineStart(int x, int y) {
  // condition 1: point is on line
  if(getTraceImageValue(x, y) == 0) return false;
  // condition 2: point has exactly one neighbour
  static int numNeighbours;
  static vector<Neighbour> *neighbourhood;
  neighbourhood = getNeighbourhood(x, y);
  numNeighbours = 0;
  for(vector<Neighbour>::iterator n = neighbourhood->begin(); n != neighbourhood->end(); n++) {
    if(*(n->traceImagePtr)) {
      numNeighbours++;
      if(numNeighbours > 1)
        return false;
    }
  }
  if(numNeighbours == 1) return true;
  else return false;
}

unsigned char *IsolineExtractor::traceImagePtr(int x, int y) {
  return traceImage + y * w + x;
}

unsigned char IsolineExtractor::getTraceImageValue(int x, int y) {
  return *(traceImagePtr(x, y));
}

void IsolineExtractor::buildNeighbourhoods() {
  static int x0, y0, x1, y1, x2, y2, nx, ny;

	neighbourhoods = new vector<Neighbour> [w * h];
	vector<Neighbour> *neighbourhoodP = neighbourhoods;
	Neighbour n;
  for(y0 = 0; y0 < h; y0++) {
    for(x0 = 0; x0 < w; x0++) {
			if(x0 == 0) x1 = 0; else x1 = x0 - 1;
			if(y0 == 0) y1 = 0; else y1 = y0 - 1;
			if(x0 == (w - 1)) x2 = w - 1; else x2 = x0 + 1;
			if(y0 == (h - 1)) y2 = h - 1; else y2 = y0 + 1;
			for(ny = y1; ny <= y2; ny++) {
				for(nx = x1; nx <= x2; nx++) {
					if(nx != x0 || ny != y0) {
						n.x = nx;
						n.y = ny;
						n.traceImagePtr = traceImagePtr(nx, ny);
						n.inputMapPtr = getInputMapPtr(nx, ny);
						neighbourhoodP->push_back(n);
					}
				}
			}
			neighbourhoodP++;
		}
	}
}

vector<IsolineExtractor::Neighbour> *IsolineExtractor::getNeighbourhood(int x, int y) {
	return neighbourhoods + (y * w) + x;
}

bool IsolineExtractor::findNeighbour(const Pixel &v1, Pixel &v2) {
  static vector<Neighbour> *neighbourhood;
  neighbourhood = getNeighbourhood(v1.x, v1.y);
  for(vector<Neighbour>::iterator n = neighbourhood->begin(); n != neighbourhood->end(); n++) {
    if(*(n->traceImagePtr)) {
			// found a neighbour - there could theoretically be more (branching), but we assume only one
			v2.x = n->x;
			v2.y = n->y;
			*(n->traceImagePtr) = 0;
			return true;
    }
  }
  return false;
}

bool IsolineExtractor::thresholdEdge(int x, int y) {
  static unsigned char v;
  v = getThresholdValue(x, y);
  if(x < (w-1))
    if(getThresholdValue(x+1, y  ) != v) return true;
  if(y < (h-1))
    if(getThresholdValue(x  , y+1) != v) return true;
  if((x < (w-1)) && (y < (h-1)))
    if(getThresholdValue(x+1, y+1) != v) return true;
  return false;
}

unsigned char IsolineExtractor::getThresholdValue(int x, int y) {
  return thresholdImage[y * w + x];
}

unsigned char *IsolineExtractor::output(int x, int y) {
  return outputMap + (y * w) + x;
}

void IsolineExtractor::interpolate() {
  for(vector<Curve>::iterator c = curves.begin(); c != curves.end(); c++)
    interpolateCurve(*c);
}

void IsolineExtractor::interpolateCurve(Curve &curve) {
	Point point;
  curve.points.clear();
	for(vector<Pixel>::iterator p = curve.pixels.begin(); p != curve.pixels.end(); p++) {
#if 0
		point.x = (float) p->x;
		point.y = (float) p->y;
#else
		interpolatePoint(*p, point);
#endif
		curve.points.push_back(point);
	}
}

void IsolineExtractor::interpolatePoint(const Pixel &pixel, Point &point) {
	/*
	each pixel neighbour pulls the curve point towards it.
	the attraction is proportional to the neighbour's comparative proximity to the threshold value in relation to the curve point's proximity.
	*/
	static int px, py;
	static float dist, ndist, nstrength, nstrengthsum, nstrengthrel;
  static vector<Neighbour> *neighbourhood;
	static float rx, ry;
	px = pixel.x;
	py = pixel.y;
	rx = (float) px;
	ry = (float) py;
  neighbourhood = getNeighbourhood(px, py);
	dist = fabsf(getInputValue(px, py) - threshold);
	nstrengthsum = 0;
  for(vector<Neighbour>::iterator n = neighbourhood->begin(); n != neighbourhood->end(); n++) {
		ndist = fabsf(*(n->inputMapPtr) - threshold);
		nstrength = dist - ndist;
		nstrengthsum += fabsf(nstrength);
	}
	if(nstrengthsum > 0) { // most likely...
		// possible optimization: store ndist/nstrengths from previous iteration
		for(vector<Neighbour>::iterator n = neighbourhood->begin(); n != neighbourhood->end(); n++) {
			ndist = fabsf(*(n->inputMapPtr) - threshold);
			nstrength = dist - ndist;
			nstrengthrel = nstrength / nstrengthsum;
			rx += nstrengthrel * (n->x - px);
			ry += nstrengthrel * (n->y - py);
		}
	}
	point.x = 0.5f + rx*(w-1)/w;
	point.y = 0.5f + ry*(h-1)/h;
}

void IsolineExtractor::smooth(float amount) {
  for(vector<Curve>::iterator c = curves.begin(); c != curves.end(); c++)
    smoothCurve(*c, amount);
}

void IsolineExtractor::smoothCurve(Curve &curve, float amount) {
  int n = 0, nLast = (int) (curve.points.size()) - 2;
  if(nLast < 2) return;
  static multimap< float, vector<Point>::iterator > sorted;
  static multimap< float, vector<Point>::iterator >::iterator sortedP;
  static float v, mx, my, dx, dy, bmdist, acdist;
  static vector<Point> *points;
  sorted.clear();
  points = &curve.points;

  vector<Point>::iterator a = points->begin();
  vector<Point>::iterator b = a; b++;
  vector<Point>::iterator c = b; c++;
  while(true) {
    mx = (a->x + c->x) / 2;
    my = (a->y + c->y) / 2;
    dx = mx - b->x;
    dy = my - b->y;
    bmdist = dx*dx + dy*dy;

    dx = a->x - c->x;
    dy = a->y - c->y;
    acdist = dx*dx + dy*dy;

    v = - bmdist / acdist;
    sorted.insert(pair< float, vector<Point>::iterator >(v, b));

    n++;
    if(n == nLast) break;
    a++; b++; c++;
  }

  int numToSmooth = (int) (amount * curve.points.size());
  sortedP = sorted.begin();
  for(n = 0; n < numToSmooth; n++) {
    b = sortedP->second;
    a = b; a--;
    c = b; c++;
    b->x = (a->x + c->x) / 2;
    b->y = (a->y + c->y) / 2;

    sortedP++;
  }
}
