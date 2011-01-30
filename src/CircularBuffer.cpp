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

#include "CircularBuffer.hpp"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

using namespace sonotopy;

template <class T>
CircularBuffer<T>::CircularBuffer(unsigned long _size) {
  size = _size;
  buffer = new T[size];
  memset(buffer, 0, sizeof(T) * size);
  writePtr = buffer;
  writePos = 0;
  readPtr = buffer;
  readPos = 0;
}

template <class T>
CircularBuffer<T>::~CircularBuffer() {
  if(size > 0)
    delete [] buffer;
}

template <class T>
void CircularBuffer<T>::write(unsigned long numItems, const T *items) {
  T *readPtr1 = (T *) items;
  for(unsigned long i = 0; i < numItems; i++) {
    write(*readPtr1++);
  }
}

template <class T>
void CircularBuffer<T>::write(const T item) {
  *writePtr++ = item;
  writePos++;
  if(writePos == size) {
    writePos = 0;
    writePtr = buffer;
  }
}

template <class T>
void CircularBuffer<T>::read(unsigned long numItems, T *dest) {
  T *destPtr = dest;
  T *readPtr1 = readPtr;
  unsigned long readPos1 = readPos;
  for(unsigned long i = 0; i < numItems; i++) {
    *destPtr++ = *readPtr1++;
    readPos1++;
    if(readPos1 == size) {
      readPos1 = 0;
      readPtr1 = buffer;
    }
  }
}

template <class T>
void CircularBuffer<T>::moveReadHead(unsigned long x) {
  readPos = (readPos + x) % size;
  readPtr = buffer + readPos;
}

template class CircularBuffer<float>;
template class CircularBuffer<double>;
template class CircularBuffer<int>;
