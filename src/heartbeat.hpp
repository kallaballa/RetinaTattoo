#ifndef HEARTBEAT_HPP_
#define HEARTBEAT_HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>

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

#endif /* HEARTBEAT_HPP_ */
