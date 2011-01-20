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

#include "Stopwatch.hpp"
#include <stdexcept>

#ifdef WIN32
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

using namespace sonotopy;

Stopwatch::Stopwatch() {
  elapsedMilliseconds = 0;
  startTime = 0;
  running = false;
}

void Stopwatch::start() {
  startTime = getWalltime();
  running = true;
}

void Stopwatch::stop() {
  elapsedMilliseconds = getWalltime() - startTime;
}

bool Stopwatch::isRunning() {
  return running;
}

unsigned long Stopwatch::getElapsedMilliseconds() {
  if(running) return getWalltime() - startTime;
  else return elapsedMilliseconds;
}

unsigned long Stopwatch::getWalltime() {
#ifdef WIN32
  struct __timeb64 timebuffer;
  _ftime64_s(&timebuffer);
  return (unsigned long) (timebuffer.time * 1000 + timebuffer.millitm);
#else
  struct timeval timebuffer;
  if(gettimeofday(&timebuffer, 0) == 0) {
    return (unsigned long) (timebuffer.tv_sec * 1000 + timebuffer.tv_usec / 1000);
  }
  else {
    throw std::runtime_error("gettimeofday failed");
  }
#endif
}
