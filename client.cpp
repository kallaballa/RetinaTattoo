#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::udp;
using std::string;

void printUsage() {
  std::cerr << "Usage: client [-f <frameSize>] <host> <port>" << std::endl;
}

int main(int argc, char* argv[])
{
  try
  {
    char c;
    size_t frameSize;
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

    for(;;) {
      boost::asio::io_service io_service;
      udp::socket socket(io_service);
      socket.open(udp::v4());

      boost::array<char, 1> send_buf  = { 0 };

      udp::resolver resolver(io_service);
      udp::resolver::query query(udp::v4(),host, port);
      udp::endpoint receiver_endpoint = *resolver.resolve(query);
      socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

      udp::socket::native_type native_sock = socket.native();
      int sendBufferSize = frameSize * 2;
      setsockopt(native_sock, SOL_SOCKET, SO_RCVBUF, &sendBufferSize, sizeof(sendBufferSize));

      std::vector<char> recv_buf;
      udp::endpoint sender_endpoint;

      // don't disable buffering on the spidev
      std::ofstream out("/dev/spidev0.0");

      clock_t pStart, period, lastPeriod = 0;
      size_t pCnt = 0, pLen = 10;

      while(socket.is_open()) {
        pStart = clock();
        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
        out.write(recv_buf.data(), len);
        //flush frame wise
        out.flush();
        boost::this_thread::sleep( boost::posix_time::milliseconds(5) );
        period += clock() - pStart;
        if(++pCnt == pLen) {
          std::cout << "fps: " << (1000.0 / ((lastPeriod + period) / 2)) / pLen << std::endl;
        }
      }
    }
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
