#include <stdio.h>
#include <ctime>
#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using std::string;

int main(int argc, char** argv)
{
  try {
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

    size_t port = boost::lexical_cast<size_t>(argv[optind]);

    //prepare the udp socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));
    std::cin.setf(std::ios_base::unitbuf);

    //set send buffer size to 2 frames
    udp::socket::native_type native_sock = socket.native();
    int buffsize = frameSize * 2;
    setsockopt(native_sock, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize));

    udp::endpoint remote_endpoint;
    boost::system::error_code error;
    boost::array<char, 1> recv_buf;

    socket.receive_from(boost::asio::buffer(recv_buf),
        remote_endpoint, 0, error);

    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error);

    char buffer[frameSize];
    for (;;) {
      boost::system::error_code ignored_error;
      std::cin.read(buffer, frameSize);
      socket.send_to(boost::asio::buffer(buffer),remote_endpoint, 0, ignored_error);
      boost::this_thread::sleep( boost::posix_time::milliseconds(5) );
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
