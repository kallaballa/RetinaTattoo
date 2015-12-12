/*
 * Mapping.hpp
 *
 *  Created on: Dec 10, 2015
 *      Author: elchaschab
 */

#ifndef MAPPING_HPP_
#define MAPPING_HPP_

#include <string>
#include <vector>
#include <iostream>
#include <map>


using std::vector;
using std::string;

struct Coordinate {
  size_t x_;
  size_t y_;
  Coordinate(): x_(0), y_(0) {

  }
  Coordinate(size_t x, size_t y) : x_(x), y_(y) {
  }
};

typedef std::vector<size_t> LedPositions;

struct CoordComp : public std::binary_function<Coordinate, Coordinate, bool>
{
    bool operator()(const Coordinate& lhs, const Coordinate& rhs) const
    {
      return lhs.x_ == rhs.x_ ? lhs.y_ < rhs.y_ : lhs.x_ < rhs.x_;
    }
};

class LedMapping : public std::map<Coordinate, LedPositions, CoordComp> {
private:
  size_t maxLedIndex_;
  size_t width_;
  size_t height_;
public:
  LedMapping() : maxLedIndex_(0), width_(0), height_(0) {
  }

  LedMapping(size_t maxLedIndex, size_t w, size_t h) : maxLedIndex_(maxLedIndex), width_(w), height_(h) {
  }

  size_t maxLedIndex() {
    return maxLedIndex_;
  }

  size_t width() {
    return width_;
  }

  size_t height() {
    return height_;
  }

  void check();
};

LedMapping readMappingFile(const string& filename);

#endif /* MAPPING_HPP_ */
