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
    if(argc - optind < 1)
      printUsage();

    size_t port = boost::lexical_cast<size_t>(argv[optind]);
    cerr << "listen: " << port << " ..." << endl;

    for(;;) {
      boost::asio::io_service io_service;
      udp::socket socket(io_service, udp::endpoint(udp::v4(), port));

      udp::socket::native_type native_sock = socket.native();
      int sendBufferSize = frameSize * 2;
      setsockopt(native_sock, SOL_SOCKET, SO_RCVBUF, &sendBufferSize, sizeof(sendBufferSize));

      std::vector<char> recv_buf;
      recv_buf.reserve(frameSize);
      udp::endpoint sender_endpoint;

      socket.receive_from(boost::asio::buffer(recv_buf.data(),1), sender_endpoint);

      HeartbeatSender heartbeat(&socket, sender_endpoint);
      boost::thread(&HeartbeatSender::run, heartbeat);

      // don't disable buffering on the spidev
      std::ofstream out("/dev/spidev0.0");
      Fps fps;
      float rate = 1;
      size_t fpsPrintLimit  = 100;

      cerr << "receive from: " << sender_endpoint << endl;
      fps.start();


      for(;;) {
        boost::thread receiverThread([&]() {
          socket.receive_from(boost::asio::buffer(recv_buf.data(),frameSize), sender_endpoint);
        });

        if(receiverThread.timed_join(1000)) {
          cerr << "stall client: " << sender_endpoint << endl;
          break;
        }

        out.write(recv_buf.data(), frameSize);
        //flush frame wise
        out.flush();
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
