#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "fps.h"

using namespace boost::posix_time;
using namespace boost::this_thread;
using boost::asio::ip::udp;
using std::string;
using std::cerr;
using std::endl;

void printUsage() {
  cerr << "Usage: client [-f <frameSize>] <host> <port>" << endl;
}

class HeartbeatSender {
  udp::socket* socket;
  udp::endpoint receiverEndpoint;
  boost::array<char, 1> buf;
public:
  HeartbeatSender(udp::socket* socket, const udp::endpoint& receiverEndpoint) :
    socket(socket),
    receiverEndpoint(receiverEndpoint) {
        buf[0] = 0;
    }
  void run() {
    for(;;) {
      socket->send_to(boost::asio::buffer(buf), receiverEndpoint);
      boost::this_thread::sleep(milliseconds(300));
    }
  }
};

int main(int argc, char** argv)
{
  try
  {
    int8_t c;
    size_t frameSize = 480;
    while ((c = getopt(argc, argv, "f:")) != -1) {
      switch (c) {
      case 'f':
        frameSize = boost::lexical_cast<size_t>(optarg);
        break;
      case ':':
        printUsage();
        break;
      case '?':
        printUsage();
        break;
      }
    }
    if(argc - optind < 2)
      printUsage();

    string host = argv[optind];
    string port = argv[optind + 1];

    cerr << "initialize: " << host << ":" << port << " ..." << endl;
    for(;;) {
      boost::asio::io_service io_service;
      udp::socket socket(io_service);
      socket.open(udp::v4());

      udp::resolver resolver(io_service);
      udp::resolver::query query(udp::v4(),host, port);
      udp::endpoint receiverEndpoint = *resolver.resolve(query);

      udp::socket::native_type native_sock = socket.native();
      int sendBufferSize = frameSize * 2;
      setsockopt(native_sock, SOL_SOCKET, SO_RCVBUF, &sendBufferSize, sizeof(sendBufferSize));

      HeartbeatSender heartbeat(&socket, receiverEndpoint);
      boost::thread(&HeartbeatSender::run, heartbeat);
      std::vector<char> recv_buf;
      recv_buf.reserve(frameSize);

      udp::endpoint sender_endpoint;

      // don't disable buffering on the spidev
      std::ofstream out("/dev/spidev0.0");
      Fps fps;
      float rate = 1;
      size_t fpsPrintLimit  = 100;

      cerr << "receive..." << endl;
      fps.start();
      while(socket.is_open()) {
        socket.receive_from(boost::asio::buffer(recv_buf.data(),frameSize), sender_endpoint);
        out.write(recv_buf.data(), frameSize);
        //flush frame wise
        out.flush();
        sleep( milliseconds(1) );

        if(fps.next() >= fpsPrintLimit) {
          rate = (fps.sample() + rate) / 2;
          cerr << "\rfps: " << rate;
        }
      }
    }
  }
  catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
