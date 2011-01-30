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

#ifndef _CircularBuffer_hpp_
#define _CircularBuffer_hpp_

namespace sonotopy {

template <class T>
class CircularBuffer {
public:
  CircularBuffer(unsigned long size);
  ~CircularBuffer();
  void write(unsigned long n, const T *); // put n items at the end and move the writehead forward
  void write(const T); // put one item at the end and move the writehead forward
  void read(unsigned long n, T *); // read n items from the readhead, without moving the readhead forward
  void moveReadHead(unsigned long n); // move the readhead forward n items

private:
  unsigned long size;
  unsigned long writePos;
  unsigned long readPos;
  T *buffer;
  T *writePtr;
  T *readPtr;
};

}

#endif
