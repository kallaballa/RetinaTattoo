#ifndef FPS_HPP_
#define FPS_HPP_

#include <boost/thread.hpp>
using boost::posix_time::ptime;
using boost::posix_time::time_period;
using boost::posix_time::microsec_clock;
using boost::this_thread::sleep;

class Fps {

  ptime pStart;
  time_period period;
  size_t frameCnt;

public:
  Fps() :
    period(ptime(), ptime()),
    frameCnt(0) {
  }

  void start() {
    this->pStart = microsec_clock::local_time();
    this->period = time_period(this->pStart,this->pStart);
  }

  const size_t&  next() {
    ++this->frameCnt;
    return this->frameCnt;
  }

  const size_t&  count() {
    return this->frameCnt;
  }

  const time_period duration() {
    return time_period(this->pStart, microsec_clock::local_time());
  }

  float sample() {
    this->period = duration();
    float millis = period.length().total_milliseconds();
    float fps = 1000.0 / (millis / this->frameCnt);
    this->frameCnt=0;
    this->pStart = microsec_clock::local_time();
    return fps;
  }
};


#endif /* FPS_HPP_ */
