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

#include <unittest++/UnitTest++.h>
#include <stdlib.h>
#include <time.h>
#include <sonotopy/sonotopy.hpp>
#include <sonotopy/TwoDimArray.hpp>
#include "math.h" // M_PI
#include <algorithm>
#include <stdio.h>

using namespace sonotopy;

TEST(CircularBuffer)
{
  CircularBuffer<int> b(5);
  // expecting rw0 0 0 0 0 (where "r" is the readhead and "w" is the writehead)
  int out[5];
  b.read(4, out);
  CHECK_EQUAL(0, out[0]);
  CHECK_EQUAL(0, out[1]);
  CHECK_EQUAL(0, out[2]);
  CHECK_EQUAL(0, out[3]);

  int in1[4] = { 11, 12, 13, 14 };
  b.write(4, in1);
  // expecting r11 12 13 14 w0
  b.read(4, out);
  CHECK_EQUAL(11, out[0]);
  CHECK_EQUAL(12, out[1]);
  CHECK_EQUAL(13, out[2]);
  CHECK_EQUAL(14, out[3]);
  b.moveReadHead(4);
  // expecting 11 12 13 14 rw0
  b.read(4, out);
  CHECK_EQUAL( 0, out[0]);
  CHECK_EQUAL(11, out[1]);
  CHECK_EQUAL(12, out[2]);
  CHECK_EQUAL(13, out[3]);
  b.moveReadHead(4);
  // expecting 11 12 13 r14 w0

  int in2[4] = { 15, 16, 17, 18 };
  b.write(4, in2);
  // expecting 16 17 18 rw14 15
  b.read(4, out);
  CHECK_EQUAL(14, out[0]);
  CHECK_EQUAL(15, out[1]);
  CHECK_EQUAL(16, out[2]);
  CHECK_EQUAL(17, out[3]);
  b.moveReadHead(4);
  // expecting 16 17 r18 w14 15
  b.read(4, out);
  CHECK_EQUAL(18, out[0]);
  CHECK_EQUAL(14, out[1]);
  CHECK_EQUAL(15, out[2]);
  CHECK_EQUAL(16, out[3]);
}

TEST(TwoDimArray)
{
  // setting and getting elements
  TwoDimArray<int> arr(3, 2);
  arr.set(0, 0, 10000);
  arr.set(1, 0, 10010);
  arr.set(2, 0, 10020);
  arr.set(0, 1, 10001);
  arr.set(1, 1, 10011);
  arr.set(2, 1, 10021);

  CHECK_EQUAL(10000, arr.get(0, 0));
  CHECK_EQUAL(10010, arr.get(1, 0));
  CHECK_EQUAL(10020, arr.get(2, 0));
  CHECK_EQUAL(10001, arr.get(0, 1));
  CHECK_EQUAL(10011, arr.get(1, 1));
  CHECK_EQUAL(10021, arr.get(2, 1));

  // iteration
  TwoDimArray<int>::Iterator i = arr.begin();
  CHECK_EQUAL(0, i->row);
  CHECK_EQUAL(0, i->column);
  CHECK_EQUAL(10000, *(i->value));
  CHECK(i != arr.end());
  ++i;
  CHECK_EQUAL(0, i->row);
  CHECK_EQUAL(1, i->column);
  CHECK_EQUAL(10001, *(i->value));
  CHECK(i != arr.end());
  ++i;
  CHECK_EQUAL(1, i->row);
  CHECK_EQUAL(0, i->column);
  CHECK_EQUAL(10010, *(i->value));
  CHECK(i != arr.end());
  ++i;
  CHECK_EQUAL(1, i->row);
  CHECK_EQUAL(1, i->column);
  CHECK_EQUAL(10011, *(i->value));
  CHECK(i != arr.end());
  ++i;
  CHECK_EQUAL(2, i->row);
  CHECK_EQUAL(0, i->column);
  CHECK_EQUAL(10020, *(i->value));
  CHECK(i != arr.end());
  ++i;
  CHECK_EQUAL(2, i->row);
  CHECK_EQUAL(1, i->column);
  CHECK_EQUAL(10021, *(i->value));
  ++i;
  CHECK(i == arr.end());

  // setting row
  int row[] = { 50010, 50011 };
  arr.setRow(1, row);
  CHECK_EQUAL(50010, arr.get(1, 0));
  CHECK_EQUAL(50011, arr.get(1, 1));

  // getting row
  const int *rowGot = arr.getRow(1);
  CHECK_EQUAL(50010, rowGot[0]);
  CHECK_EQUAL(50011, rowGot[1]);

  // fill
  arr.fill(10000);
  CHECK_EQUAL(10000, arr.get(0, 0));
  CHECK_EQUAL(10000, arr.get(1, 0));
  CHECK_EQUAL(10000, arr.get(2, 0));
  CHECK_EQUAL(10000, arr.get(0, 1));
  CHECK_EQUAL(10000, arr.get(1, 1));
  CHECK_EQUAL(10000, arr.get(2, 1));
}

float sampleDistance(SOM::Sample p, SOM::Sample q) {
  return (float) (::sqrt((p[0]-q[0])*(p[0]-q[0]) + (p[1]-q[1])*(p[1]-q[1]))) / ::sqrt((float)2);
}

float modelNeighbourStrength(int dx, int dy) {
  float strength;
  int dist = dx*dx + dy*dy;
  if(dist < 2*2)
    strength = 1.0f - (float) dist / (2*2);
  else
    strength = 0;
  return strength;
}

#define setSomModel(x, y, value) (net.setModel(topology.gridCoordinatesToId(x, y), value))
#define getSomOutput(x, y) (output[topology.gridCoordinatesToId(x, y)])

TEST(RectGridSOM) {
  /*
  n n n   (n = normal model   W = winner model)
  n n W
  n n n
  */
  unsigned int inputSize = 2;
  unsigned int gridWidth = 3;
  unsigned int gridHeight = 3;
  double normalModelValue[] = { 0.5, 0.5 };
  double inputValue[]       = { 0.9, 0.8 };
  double winnerModelValue[] = { 0.6, 0.6 };
  float precision = 0.0001f;
  unsigned int winnerId;
  SOM::Output output;
  RectGridTopology::Node winner;

  RectGridTopology topology(gridWidth, gridHeight);
  SOM net(inputSize, &topology);
  SOM::Sample normalModel = net.createSample(normalModelValue);
  SOM::Sample input = net.createSample(inputValue);
  SOM::Sample winnerModel = net.createSample(winnerModelValue);

  setSomModel(0, 0, normalModel);
  setSomModel(0, 1, normalModel);
  setSomModel(0, 2, normalModel);
  setSomModel(1, 0, normalModel);
  setSomModel(1, 1, normalModel);
  setSomModel(1, 2, normalModel);
  setSomModel(2, 0, normalModel);
  setSomModel(2, 1, winnerModel);
  setSomModel(2, 2, normalModel);

  net.setLearningParameter(0.5);
  net.setNeighbourhoodParameter(0.5);

  net.train(input);
  winnerId = net.getLastWinner();
  winner = topology.getNode(winnerId);
  net.getLastOutput(output);

  // FIRST TRIAL
  // (2,1) is the winner
  CHECK_EQUAL(2, winner.x);
  CHECK_EQUAL(1, winner.y);
  // all non-winner models have higher output
  net.getLastOutput(output);
  CHECK(getSomOutput(2,1) < getSomOutput(0,0));
  CHECK(getSomOutput(2,1) < getSomOutput(0,1));
  CHECK(getSomOutput(2,1) < getSomOutput(0,2));
  CHECK(getSomOutput(2,1) < getSomOutput(1,0));
  CHECK(getSomOutput(2,1) < getSomOutput(1,1));
  CHECK(getSomOutput(2,1) < getSomOutput(1,2));
  CHECK(getSomOutput(2,1) < getSomOutput(2,0));
  CHECK(getSomOutput(2,1) < getSomOutput(2,2));
  // all non-winner models have same output
  CHECK_CLOSE(getSomOutput(0,1), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(0,2), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(1,0), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(1,1), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(1,2), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(2,0), getSomOutput(0,0), precision);
  CHECK_CLOSE(getSomOutput(2,2), getSomOutput(0,0), precision);

  // SECOND TRIAL
  net.train(input);
  winnerId = net.getLastWinner();
  winner = topology.getNode(winnerId);
  net.getLastOutput(output);
  // (2,1) is still the winner
  CHECK_EQUAL(2, winner.x);
  CHECK_EQUAL(1, winner.y);
  // winner has lower output than its directly adjacent (+1+0) neighbours (2,0) (2,2) & (1,1)
  CHECK(getSomOutput(2,1) < getSomOutput(2,0));
  CHECK(getSomOutput(2,1) < getSomOutput(2,2));
  CHECK(getSomOutput(2,1) < getSomOutput(1,1));
  // the directly adjacent (+1+0) neighbours (2,0) (2,2) & (1,1) have equal output
  CHECK_CLOSE(getSomOutput(2,0), getSomOutput(2,2), precision);
  CHECK_CLOSE(getSomOutput(2,0), getSomOutput(1,1), precision);
  // the directly adjacent (+1+0) neighbours (2,0) (2,2) & (1,1) have lower output than the diagonal (+1+1) neighbours (1,0) (1,2)
  CHECK(getSomOutput(2,0) < getSomOutput(1,0));
  CHECK(getSomOutput(2,2) < getSomOutput(1,0));
  CHECK(getSomOutput(1,1) < getSomOutput(1,0));
  // the diagonal (+1+1) neighbours (1,0) (1,2) have equal output
  CHECK_CLOSE(getSomOutput(1,0), getSomOutput(1,2), precision);
  // the diagonal (+1+1) neighbours (1,0) (1,2) have lower output than the (+2+0) neighbour (0,1)
  CHECK(getSomOutput(1,0) < getSomOutput(0,1));
  CHECK(getSomOutput(1,2) < getSomOutput(0,1));
  // the (+2+0) neighbour (0,1) has lower output than the (+2+1) neighbours (0,0) (0,2)
  CHECK(getSomOutput(2,0) < getSomOutput(0,0));
  CHECK(getSomOutput(2,0) < getSomOutput(0,2));
  // the (+2+1) neighbours (0,0) (0,2) have equal output
  CHECK_CLOSE(getSomOutput(0,0), getSomOutput(0,2), precision);

  // winner output is not zero since learning parameter is only 0.5
  CHECK(getSomOutput(2,1) > precision);

  // winner output is zero after training with learning parameter 1.0
  net.setLearningParameter(1.0);
  net.train(input);
  net.train(input);
  net.getLastOutput(output);
  CHECK(getSomOutput(2,1) < precision);
}

TEST(StressTestSOM) {
  // create SOM with random models
  int numIterations = 1000;
  unsigned int inputSize = 3;
  unsigned int gridWidth = 2;
  unsigned int gridHeight = 2;
  unsigned int winnerId;
  RectGridTopology topology(gridWidth, gridHeight);
  SOM net(inputSize, &topology);
  srand((unsigned int) time(NULL));
  net.setRandomModelValues();

  int *winnerCount = new int [topology.getNumNodes()];
  for(unsigned int i = 0; i < topology.getNumNodes(); i++)
    winnerCount[i] = 0;

  // train it with random input
  SOM::Sample input;
  SOM::Output output;
  for(int i = 0; i < numIterations; i++) {
    input.clear();
    for(unsigned int j = 0; j < inputSize; j++)
      input.push_back((float) rand() / RAND_MAX);

    net.train(input);
    net.getLastOutput(output);
    winnerId = net.getLastWinner();
    winnerCount[winnerId]++;
  }

  // output winner distribution
  printf("StressTestSOM winner distribution (should be fairly equal):\n");
  for(unsigned int i = 0; i < topology.getNumNodes(); i++)
    printf("%d: %d\n", i, winnerCount[i]);
}

TEST(CircleSOM) {
  /*
      0
   7     1
  6       2
   5     3
      4
  */
  unsigned int numNodes = 8;
  float precision = 0.0001f;
  CircleTopology topology(numNodes);

  // verify node angles
  CHECK_CLOSE(2*M_PI*0/8, topology.getNode(0).angle, precision);
  CHECK_CLOSE(2*M_PI*1/8, topology.getNode(1).angle, precision);
  CHECK_CLOSE(2*M_PI*2/8, topology.getNode(2).angle, precision);
  CHECK_CLOSE(2*M_PI*3/8, topology.getNode(3).angle, precision);
  CHECK_CLOSE(2*M_PI*4/8, topology.getNode(4).angle, precision);
  CHECK_CLOSE(2*M_PI*5/8, topology.getNode(5).angle, precision);
  CHECK_CLOSE(2*M_PI*6/8, topology.getNode(6).angle, precision);
  CHECK_CLOSE(2*M_PI*7/8, topology.getNode(7).angle, precision);

  // distances from node 0
  CHECK_CLOSE(topology.getDistance(0, 1),  topology.getDistance(0, 7), precision);
  CHECK(      topology.getDistance(0, 1) < topology.getDistance(0, 6));
  CHECK_CLOSE(topology.getDistance(0, 6),  topology.getDistance(0, 2), precision);
  CHECK      (topology.getDistance(0, 6) < topology.getDistance(0, 5));
  CHECK_CLOSE(topology.getDistance(0, 5),  topology.getDistance(0, 3), precision);
  CHECK(      topology.getDistance(0, 5) < topology.getDistance(0, 4));

  // distances from node 1
  CHECK_CLOSE(topology.getDistance(1, 0),  topology.getDistance(1, 2), precision);
  CHECK(      topology.getDistance(1, 0) < topology.getDistance(1, 7));
  CHECK_CLOSE(topology.getDistance(1, 7),  topology.getDistance(1, 3), precision);
  CHECK      (topology.getDistance(1, 7) < topology.getDistance(1, 6));
  CHECK_CLOSE(topology.getDistance(1, 6),  topology.getDistance(1, 4), precision);
  CHECK(      topology.getDistance(1, 6) < topology.getDistance(1, 5));

  // distances from node 4
  CHECK_CLOSE(topology.getDistance(4, 5),  topology.getDistance(4, 3), precision);
  CHECK(      topology.getDistance(4, 5) < topology.getDistance(4, 6));
  CHECK_CLOSE(topology.getDistance(4, 6),  topology.getDistance(4, 2), precision);
  CHECK      (topology.getDistance(4, 6) < topology.getDistance(4, 7));
  CHECK_CLOSE(topology.getDistance(4, 7),  topology.getDistance(4, 1), precision);
  CHECK(      topology.getDistance(4, 7) < topology.getDistance(4, 0));
}

TEST(SpectrumMap) {
  unsigned int gridWidth = 3;
  unsigned int gridHeight = 3;
  int spectrumResolution = 10;
  RectGridTopology topology(gridWidth, gridHeight);
  SpectrumMap spectrumMap(&topology, spectrumResolution);
  SpectrumMap::ActivationPattern *activationPattern = spectrumMap.createActivationPattern();
  CHECK_EQUAL((size_t)9, activationPattern->size());

  spectrumMap.getActivationPattern(activationPattern);
  CHECK_EQUAL((size_t)9, activationPattern->size());
}

TEST(GridMapCircuit_repeat_getWinnerPosition) {
  AudioParameters audioParameters;
  GridMapCircuitParameters gridMapCircuitParameters;
  GridMapCircuit gridMapCircuit(audioParameters, gridMapCircuitParameters);
  float x, y;
  float *audio = new float [audioParameters.bufferSize];
  gridMapCircuit.feedAudio(audio, audioParameters.bufferSize);
  gridMapCircuit.getWinnerPosition(x, y);
  gridMapCircuit.getWinnerPosition(x, y);
  gridMapCircuit.getWinnerPosition(x, y);
  delete [] audio;
}

TEST(NormalizerImmediate) {
  Normalizer normalizer;
  float precision = 0.0001;

  CHECK_CLOSE(1.0, normalizer.normalize(0.5), precision);
  CHECK_CLOSE(0.6, normalizer.normalize(0.3), precision);
  CHECK_CLOSE(1.0, normalizer.normalize(0.8), precision);
  CHECK_CLOSE(0.5, normalizer.normalize(0.4), precision);
  CHECK_CLOSE(0.5, normalizer.normalize(0.4), precision);
  CHECK_CLOSE(0.5, normalizer.normalize(0.4), precision);
}


TEST(NormalizerWithAdaptation) {
  Normalizer normalizer;
  normalizer.setAdaptationFactor(0.5);
  float precision = 0.0001;

  CHECK_CLOSE(1.0, normalizer.normalize(0.5), precision);
  CHECK_CLOSE(0.6, normalizer.normalize(0.3), precision);
  CHECK_CLOSE(1.0, normalizer.normalize(0.8), precision);
  CHECK_CLOSE(0.5, normalizer.normalize(0.4), precision);
  CHECK_CLOSE(0.666667, normalizer.normalize(0.4), precision);
  CHECK_CLOSE(0.8, normalizer.normalize(0.4), precision);
}

int main()
{
  return UnitTest::RunAllTests();
}
