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

#ifndef _TwoDimArray_hpp_
#define _TwoDimArray_hpp_

#include <assert.h>
#include <string.h>
#ifndef NULL
#define NULL 0
#endif

namespace sonotopy {

template <typename ContentType>
class TwoDimArray {
public:
  struct Link {
    unsigned int row;
    unsigned int column;
    ContentType* value;
    Link *next;
  };

  class Iterator {
  public:
    Iterator(Link *_current = NULL) {
      current = _current;
    }
    const Link& operator*() const {
      return *current;
    }
    const Link* operator->() const {
      return current;
    }
    Iterator& operator++() { // prefix
      current = current->next;
      return *this;
    }
    Iterator& operator++(int) { // postfix
      current = current->next;
      return *this;
    }
    bool operator==(const Iterator &other) {
      return current == other.current;
    }
    bool operator!=(const Iterator &other) {
      return current != other.current;
    }
  private:
    Link* current;
  };

  TwoDimArray(unsigned int numRows, unsigned int numColumns);
  ~TwoDimArray();
  void set(unsigned int row, unsigned int column, ContentType value);
  void setRow(unsigned int row, const ContentType *values);
  void fill(ContentType value);
  const ContentType& get(unsigned int row, unsigned int column) const;
  ContentType& get(unsigned int row, unsigned int column);
  const ContentType* getRow(unsigned int row) const;
  Iterator begin() const { return Iterator(head); }
  Iterator end() const { return Iterator(NULL); }

private:
  unsigned int numColumns;
  unsigned int numRows;
  ContentType **rows;
  Link* head;
  unsigned int size;
};



template <typename ContentType>
TwoDimArray<ContentType>::TwoDimArray(unsigned int _numRows, unsigned int _numColumns) {
  numRows = _numRows;
  numColumns = _numColumns;

  rows = new ContentType* [numRows];
  ContentType **row = rows;
  for(unsigned int r = 0; r < numRows; r++)
    *row++ = new ContentType [numColumns];

  Link *current, *previous=NULL;
  row = rows;
  ContentType *value;
  for(unsigned int r = 0; r < numRows; r++) {
    value = *row;
    for(unsigned int c = 0; c < numColumns; c++) {
      current = new Link;
      current->row = r;
      current->column = c;
      current->value = value;
      if(r== 0 && c == 0)
        head = current;
      else
        previous->next = current;
      previous = current;
      value++;
    }
    row++;
  }
  previous->next = NULL;
}

template <typename ContentType>
TwoDimArray<ContentType>::~TwoDimArray() {
  Link *current = head;
  Link *next;
  while(current != NULL) {
    next = current->next;
    delete current;
    current = next;
  }

  ContentType **row = rows;
  for(unsigned int r = 0; r < numRows; r++)
    delete [] *row++;
  delete [] rows;
}

template <typename ContentType>
void TwoDimArray<ContentType>::set(unsigned int r, unsigned int c, ContentType value) {
  assert(r < numRows);
  assert(c < numColumns);
  rows[r][c] = value;
}

template <typename ContentType>
void TwoDimArray<ContentType>::setRow(unsigned int r, const ContentType *values) {
  assert(r < numRows);
  memcpy(rows[r], values, sizeof(ContentType) * numColumns);
}

template <typename ContentType>
void TwoDimArray<ContentType>::fill(ContentType value) {
  for(Iterator i = begin(); i != end(); i++)
    *(i->value) = value;
}

template <typename ContentType>
const ContentType& TwoDimArray<ContentType>::get(unsigned int r, unsigned int c) const {
  assert(r < numRows);
  assert(c < numColumns);
  return rows[r][c];
}

template <typename ContentType>
ContentType& TwoDimArray<ContentType>::get(unsigned int r, unsigned int c) {
  assert(r < numRows);
  assert(c < numColumns);
  return rows[r][c];
}

template <typename ContentType>
const ContentType* TwoDimArray<ContentType>::getRow(unsigned int r) const {
  return rows[r];
}


}

#endif
