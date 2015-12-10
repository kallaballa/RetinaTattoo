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
  size_t x;
  size_t y;
};

typedef std::vector<size_t> LedPositions;

struct CoordComp : public std::binary_function<Coordinate, Coordinate, bool>
{
    bool operator()(const Coordinate& lhs, const Coordinate& rhs) const
    {
      return lhs.x == rhs.x ? lhs.y < rhs.y : lhs.x < rhs.x;
    }
};

class LedMapping : public std::map<Coordinate, LedPositions, CoordComp> {
private:
  size_t numLeds_;
  size_t width_;
  size_t height_;
public:
  LedMapping(size_t n, size_t w, size_t h) : numLeds_(n), width_(w), height_(h) {
  }

  size_t numLeds() {
    return numLeds_;
  }

  size_t width() {
    return width_;
  }

  size_t height() {
    return height_;
  }
};

string extractNextToken(string& s, const string& delimiter);
LedMapping readMappingFile(const string& filename);

#endif /* MAPPING_HPP_ */
