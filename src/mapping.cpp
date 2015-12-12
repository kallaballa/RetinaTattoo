#include "mapping.hpp"
#include "exception.hpp"
#include <fstream>
#include <sstream>
#include <cassert>
#include <tuple>
#include <set>
#include <algorithm>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

std::pair<string,off_t> extractToken(const string& s, size_t offset, const string& delimiter) {
   size_t pos = s.find(delimiter, offset);
  if(pos == std::string::npos) {
    return std::make_pair("", offset);
  }
  string token = s.substr(offset, pos - offset);
  return std::make_pair(token, pos + 1);
}

LedMapping parseHeader(std::istream& ifs) {
  string line;
  size_t lineNr = 1;
  std::pair<string,off_t> token;

  if(!std::getline(ifs, line))
    throw ParseException("Unable to read header (empty file?)", line, lineNr, token.second);

  //parse width
  token = extractToken(line, 0, "x");
  string strWidth = token.first;

  if(strWidth.empty())
    throw ParseException("Couldn't parse width", line, lineNr, token.second);

  size_t w = 0;
  try {
    w = lexical_cast<size_t>(strWidth);
  } catch (std::exception& ex) {
    throw ParseException("Width value not an integer", line, lineNr, token.second - strWidth.length());
  }

  //parse height
  token = extractToken(line, token.second, "=");
  string strHeight = token.first;
  if(strHeight.empty())
    throw ParseException("Couldn't parse height", line, lineNr, token.second);

  size_t h = 0;
  try {
    h = lexical_cast<size_t>(strHeight);
  } catch (std::exception& ex) {
    throw ParseException("Height value not an integer", line, lineNr, token.second - strHeight.length());
  }


  //parse maximum leds index
  string strMaximumLedIndex = line.substr(token.second, line.length() - token.second);

  if(strMaximumLedIndex.empty())
    throw ParseException("Max led index missing", line, lineNr, token.second);

  size_t maxLedIndex = 0;
  try {
    maxLedIndex = lexical_cast<size_t>(strMaximumLedIndex);
  } catch (std::exception& ex) {
    throw ParseException("Max led index value not an integer", line, lineNr, token.second);
  }

  return LedMapping(maxLedIndex,w,h);
}

LedMapping readMappingFile(const string& filename) {
  std::ifstream ifs(filename);
  string line;
  size_t lineNr = 1;
  std::pair<string,off_t> token;
  LedMapping map = parseHeader(ifs);

  while(std::getline(ifs, line)) {
    ++lineNr;
    if(line.empty())
      continue;

    //parse x
    token = extractToken(line, 0, "/");
    string strX = token.first;
    if(strX.empty())
      throw ParseException("Couldn't parse x", line, lineNr, token.second);

    size_t x = 0;
    try {
      x = lexical_cast<size_t>(strX);
    } catch(std::exception& ex) {
      throw ParseException("X value not an integer", line, lineNr, token.second - strX.length());
    }

    //parse y
    token = extractToken(line, token.second, "=");
    string strY = token.first;
    if(strY.empty())
      throw ParseException("Couldn't parse y", line, lineNr, token.second);

    size_t y = 0;
    try {
      y = lexical_cast<size_t>(strY);
    } catch(std::exception& ex) {
      throw ParseException("Y value not an integer", line, lineNr, token.second - strY.length());
    }
    string strLedPosList = line.substr(token.second, line.length() - token.second);
    if(strLedPosList.empty())
      throw ParseException("Empty led position list", line, lineNr, token.second);

    LedPositions ledPositions;
    size_t pos;
    size_t lastOff = token.second;
    while(!(token = extractToken(line, token.second, ",")).first.empty()) {
      try {
        pos = lexical_cast<size_t>(token.first);
      } catch(std::exception& ex) {
        throw ParseException("Led position value is not an integer", line, lineNr, token.second - token.first.length());
      }
      lastOff = token.second;
      ledPositions.push_back(pos);
    }
    if(line.length() < lastOff)
      std::cerr << "lastoff: " << lastOff << "/" << line.length() << "=" << line.length() - lastOff << std::endl;

    pos = lexical_cast<size_t>(line.substr(lastOff, line.length() - lastOff));
    ledPositions.push_back(pos);
    map[{x,y}] = ledPositions;
  }

  map.check();
  return map;
}

/*
 *  Check max index matches.
 *  Check all coordinates mapped.
 *  Check coordinates double allocated.
 *  Check all led positions used.
 * Check led positions double allocated.
 */

void LedMapping::check() {
  std::set<Coordinate, CoordComp> sCoord;
  std::set<LedPositions::value_type> sPositions;

  std::vector<Coordinate> cNotMapped;
  std::vector<Coordinate> cDoubleAlloc;

  std::vector<LedPositions::value_type> lpNotMapped;
  std::vector<LedPositions::value_type> lpDoubleAlloc;

  for(auto it : (*this)) {
    const Coordinate& coord = it.first;
    LedPositions& positions = it.second;

    if(!sCoord.insert(coord).second) {
      cDoubleAlloc.push_back(coord);
    }

    for(auto pos : positions) {
      if(!sPositions.insert(pos).second) {
        lpDoubleAlloc.push_back(pos);
      }
    }
  }

  if((*sPositions.rbegin()) != maxLedIndex())
    throw std::runtime_error("Max led index doesn't match");

  if(!cDoubleAlloc.empty()) {
    std::stringstream sscda;
    std::sort(cDoubleAlloc.begin(), cDoubleAlloc.end(),CoordComp());

    for(Coordinate& coord: cDoubleAlloc) {
      sscda << coord.x_ << "/" << coord.y_ << ", ";
    }
    string strCDoubleList = sscda.str();
    strCDoubleList = strCDoubleList.substr(0, strCDoubleList.length() - 2);
    throw std::runtime_error("Coordinates double allocated: " + strCDoubleList);
  }


  if(!lpDoubleAlloc.empty()) {
    std::stringstream sslda;
    std::sort(lpDoubleAlloc.begin(), lpDoubleAlloc.end());
    for(const LedPositions::value_type& pos: lpDoubleAlloc) {
      sslda << pos << ", ";
    }
    string strLpDoubleList = sslda.str();
    strLpDoubleList = strLpDoubleList.substr(0, strLpDoubleList.length() - 2);
    std:: cerr << "Warning: Led positions double allocated: " << strLpDoubleList << std::endl;
  }

  for(int x = 0; x < width(); ++x) {
    for(int y = 0; y < height(); ++y) {
      Coordinate c(x,y);
      if(sCoord.find(c) == sCoord.end()) {
        cNotMapped.push_back(c);
      }
    }
  }

  std::stringstream ss;
  string msg;
  if(!cNotMapped.empty()) {
  ss << "Warning: Coordinates not mapped: ";
    for(Coordinate& coord: cNotMapped) {
      ss << coord.x_ << "/" << coord.y_ << ", ";
    }

    msg = ss.str();
    msg = msg.substr(0, msg.length() - 2);
    std::cerr << msg << std::endl;
  }

  for(LedPositions::value_type i = 0; i < maxLedIndex() + 1; ++i) {
    if(sPositions.find(i) == sPositions.end()) {
      lpNotMapped.push_back(i);
    }
  }

  if(!lpNotMapped.empty()) {
    ss.str("");
    ss << "Warning: Led positions not mapped: ";
    for(LedPositions::value_type& pos: lpNotMapped) {
      ss << pos << ", ";
    }

    msg = ss.str();
    msg = msg.substr(0, msg.length() - 2);
    std::cerr << msg << std::endl;
  }
}


