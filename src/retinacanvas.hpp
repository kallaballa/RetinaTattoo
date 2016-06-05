#ifndef RETINACANVAS_HPP_
#define RETINACANVAS_HPP_

#include <string>
#include <mutex>
#include <boost/asio.hpp>
#include "color.hpp"
#include "fps.hpp"
#include "heartbeat.hpp"

namespace asio = boost::asio;
using asio::ip::udp;

using std::string;
using std::cerr;
using std::endl;

class RetinaCanvas {
  string host_;
  uint16_t port_;
  size_t width_ = 0;
  size_t height_ = 0;
  size_t frameSize_ = 0 ;
  size_t frameRate_ = 200;
  Fps fps_;
  boost::asio::io_service io_service_;
  udp::socket socket_;
  HeartbeatReceiver hearbeat_;
  std::vector<uint8_t> frameBuffer_;
  std::vector<uint8_t> rowBuffer_;
  udp::endpoint endpoint_;
  std::mutex frameBufferMutex_;
  bool update_;
public:
  RetinaCanvas(string host, uint16_t port, size_t width, size_t height, size_t frameRate);
  void connect();
  void setPixelRGB(const size_t& x, const size_t& y, const RGBColor& rgb);
  void setPixelHSL(const size_t& x, const size_t& y, const HSLColor& hsl);
  void update();
  void run();
};

#endif
