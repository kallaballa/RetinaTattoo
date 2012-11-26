#include <stdio.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <unistd.h>
#include <sys/time.h>
#include "fps.h"
#include <boost/tokenizer.hpp>

namespace asio = boost::asio;
using asio::ip::udp;

using boost::this_thread::sleep;
using boost::posix_time::milliseconds;

using std::string;
using std::cerr;
using std::endl;

void printUsage() {
  cerr << "Usage: client [-d<dimension>][-r <frameRate>][-b <sendBufferSize>]<port>" << endl;
  cerr << "[-r <frameRate>]\te.g: 24"  << endl;
  cerr << "[-d <dimension>]\te.g: 400x400" << endl;
  cerr << "[-b <sendBufferSize>]" << endl;
  cerr << "<host>\thostname to connect to" << endl;
  cerr << "<port>\tudp port of the target host" << endl;
}

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

int main(int argc, char** argv) {
  try {
    int8_t c;
    size_t width = 0;
    size_t height = 0;
    size_t frameSize = 0 ;
    size_t frameRate = 200;
    string dim = "160x1";

    while ((c = getopt(argc, argv, "r:d:f:")) != -1) {
      switch (c) {
      case 'd':
        dim = string(optarg);
        break;
      case 'r':
        frameRate = boost::lexical_cast<size_t>(optarg);
        break;
      case ':':
        printUsage();
        break;
      case '?':
        printUsage();
        break;
      }
    }

    boost::char_separator<char> ssep("x", "", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char> > tokComponents(dim, ssep);
    auto it = tokComponents.begin();
    width = boost::lexical_cast<size_t>(*it++);
    height = boost::lexical_cast<size_t>(*it);
    frameSize = width * height * 3;

    if(argc - optind < 1)
      printUsage();

    string host = argv[optind];
    string port = argv[optind + 1];

    cerr << "connecting: " << host << ":" << port << endl;

    // set cin unbuffered
    auto& in = std::cin;
//    std::ifstream in("/dev/urandom");
    in.setf(std::ios_base::unitbuf);

    //prepare the udp socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service);
    socket.open(udp::v4());

    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(),host, port);
    udp::endpoint endpoint = *resolver.resolve(query);

    //set send buffer size to 2 frames
    udp::socket::native_type native_sock = socket.native();
    int buffsize = frameSize * 2;
    setsockopt(native_sock, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize));

    for(;;) {
      boost::system::error_code error;

      HeartbeatReceiver hearbeat(&socket);
      boost::thread(&HeartbeatReceiver::run, &hearbeat);

      if (error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);

      char readBuf[frameSize];

      boost::system::error_code ignored_error;

      Fps fps;
      size_t fpsPrintLimit = 100;
      float targetDur = (1000.0 / frameRate);
      float rate = 1;
      fps.start();

      cerr << "waiting..." << endl;
      char hb[1] = { 0 };
      while (!hearbeat.alive()) {
        socket.send_to(boost::asio::buffer(hb, 1),endpoint, 0, ignored_error);
        sleep( milliseconds(1000) );
      }

      cerr << "sending..." << endl;
      boost::thread transformAndSendThread;
      char rowBuf[width * 3];

      std::cerr << "size:" << frameSize
          << " width:" << width
          << " height:" << height << std::endl;

      while (hearbeat.alive()) {
        if(transformAndSendThread.joinable())
          transformAndSendThread.join();

        in.read(readBuf, frameSize);
        bool forward = true;
        transformAndSendThread = boost::thread([&]() {
          for(size_t y = 0; y < height; y++) {
            for(size_t x = 0; x < (width * 3); x+=3) {
              size_t off = y * width * 3;

              if(!forward) {
                rowBuf[x] = readBuf[off + x];
                rowBuf[x + 1] = readBuf[off + x + 1];
                rowBuf[x + 2] = readBuf[off + x + 2];
              } else {
                rowBuf[(width*3) - x - 3] = readBuf[off + x];
                rowBuf[(width*3) - x - 2] = readBuf[off + x + 1];
                rowBuf[(width*3) - x - 1] = readBuf[off + x + 2];
              }
            }

            forward = !forward;
            memcpy(readBuf + (width * 3 * y), rowBuf, width * 3);
          }

          socket.send_to(asio::buffer(readBuf, frameSize),endpoint, 0, ignored_error);
        });

        if(fps.next() >= fpsPrintLimit) {
          float sampledRate = fps.sample();
          rate = (sampledRate + rate) / 2;
          cerr << "\rfps: " << rate;
        }

        sleep( milliseconds(targetDur));
      }

      cerr << endl << "client lost" << endl;
    }
  }
  catch (std::exception& e) {
    cerr << e.what() << endl;
  }

  return 0;
}
