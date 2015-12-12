#ifndef SRC_EXCEPTION_HPP_
#define SRC_EXCEPTION_HPP_

#define MAX_LINE_EXCERPT_LENGHT 24

#include <exception>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

using std::string;

class ParseException : public std::runtime_error {
public:
  ParseException(string msg, string line, off_t lineNr,  off_t linePos);
};


#endif
