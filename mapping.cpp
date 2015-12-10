#include "mapping.hpp"
#include <fstream>
#include <sstream>
#include <cassert>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

string extractNextToken(string& s, const string& delimiter) {
  size_t pos = s.find(delimiter);
  if(pos == std::string::npos) {
    return "";
  }
  string token = s.substr(0, pos);
  s.erase(0, pos + delimiter.length());
  return token;
}

LedMapping readMappingFile(const string& filename) {
  std::ifstream ifs(filename);
  string line;
  assert(std::getline(ifs, line));
  string strWidth = extractNextToken(line, "x");
  assert(!strWidth.empty());
  string strHeight = extractNextToken(line, "=");
  assert(!strHeight.empty());
  assert(!line.empty());

  size_t w = lexical_cast<size_t>(strWidth);
  size_t h = lexical_cast<size_t>(strHeight);
  size_t num = lexical_cast<size_t>(line);

  LedMapping map(num,w,h);

  while(std::getline(ifs, line)) {
    if(line.empty())
      continue;

    string strX = extractNextToken(line, "/");
    assert(!strX.empty());
    string strY = extractNextToken(line, "=");
    assert(!strY.empty());
    assert(!line.empty());

    size_t x = lexical_cast<size_t>(strX);
    size_t y = lexical_cast<size_t>(strY);

    string strPos;
    LedPositions ledPositions;
    size_t pos;
    while(!(strPos = extractNextToken(line, ",")).empty()) {
      pos = lexical_cast<size_t>(strPos);
      ledPositions.push_back(pos);
    }
    pos = lexical_cast<size_t>(line);
    ledPositions.push_back(pos);
    map[{x,y}] = ledPositions;
  }
  //check num of leds!!
  return map;
}
