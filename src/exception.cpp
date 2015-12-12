#include "exception.hpp"

string makeParseErrorMessage(string msg, string line, size_t lineNr, size_t linePos) {
  string lineInfo = boost::lexical_cast<string>(lineNr) + ":" + boost::lexical_cast<string>(linePos) + " ";
  std::stringstream ss;
  ss << msg << std::endl;
  int32_t start = 0;
  int32_t end = line.length();
  if(line.length() > MAX_LINE_EXCERPT_LENGHT) {
    start = linePos - (MAX_LINE_EXCERPT_LENGHT / 2);
    end  = linePos + (MAX_LINE_EXCERPT_LENGHT / 2);
    if(start < 0) {
      end = linePos + (MAX_LINE_EXCERPT_LENGHT - (start * -1));
      start = 0;
    } else {
      linePos = linePos - start;
    }

    line = line.substr(start, end - start);
  }
  ss << lineInfo;
  if((uint32_t)start > 0)
    ss << "...";

  ss << line;

  if((uint32_t)end < line.length())
    ss << "...";

  ss << std::endl;

  return ss.str();
}

ParseException::ParseException(string msg, string line, off_t lineNr,  off_t linePos) : std::runtime_error(makeParseErrorMessage(msg, line, lineNr, linePos)) {
}

