#include "heartbeat.hpp"

HeartbeatSender::HeartbeatSender(udp::socket* socket, const udp::endpoint& receiverEndpoint) :
  socket(socket),
  receiverEndpoint(receiverEndpoint) {
      buf[0] = 0;
}

void HeartbeatSender::run() {
  for(;;) {
    socket->send_to(boost::asio::buffer(buf), receiverEndpoint);
    boost::this_thread::sleep(milliseconds(300));
  }
}
