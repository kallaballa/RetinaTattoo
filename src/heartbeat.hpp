#ifndef HEARTBEAT_HPP_
#define HEARTBEAT_HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace asio = boost::asio;
using asio::ip::udp;

using namespace boost::posix_time;
using boost::asio::ip::udp;

class HeartbeatSender {
  udp::socket* socket;
  udp::endpoint receiverEndpoint;
  char buf[1];
public:
  HeartbeatSender(udp::socket* socket, const udp::endpoint& receiverEndpoint);
  void run();
};

class HeartbeatReceiver {
  udp::socket* socket;
  char buf[1];
  boost::posix_time::ptime last;
  bool alive_;
  bool checked_;
public:
  HeartbeatReceiver(udp::socket* socket) :
    socket(socket),
    alive_(false),
    checked_(false)
  {}

  void run() {
    using namespace boost::posix_time;
    boost::system::error_code error;
    ptime now;
    udp::endpoint endpoint;
    while(!this->checked_ || this->alive_) {
      socket->receive_from(asio::buffer(buf), endpoint, 0, error);

      if(error == boost::system::errc::success) {
        last = microsec_clock::local_time();
      }
    }
  }

  bool alive() {
    ptime now = microsec_clock::local_time();
    if((now - last).total_milliseconds() < 1000) {
      this->alive_ = true;
      this->checked_ = true;
    } else {
      this->alive_ = false;
    }

    return this->alive_;
  }
};

#endif /* HEARTBEAT_HPP_ */
