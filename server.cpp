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
#include "color.h"
#include <boost/tokenizer.hpp>

using namespace boost::posix_time;
using namespace boost::this_thread;
using boost::asio::ip::udp;
using std::string;
using std::cerr;
using std::endl;

void printUsage() {
  cerr << "Usage: server [-a][-o<output device>][-d<dimension>][-h<hue>][-s<saturation>][-l<lightness>][-f <pixFormat>] <port>" << endl;
  cerr << "[-h <hue>]\t hue modifier either an offset in degrees or a floating point scale" << endl;
  cerr << "[-d <dimension>]\te.g: 400x400" << endl;
  cerr << "[-h <hue>]\t0 to 360" << endl;  
  cerr << "[-s <saturation>]\t-100 to 100" << endl;
  cerr << "[-l <lightness>]\t-100 to 100" << endl;
  cerr << "[-a]\talternate scan order" << endl;
  cerr << "[-f]\t Pixel format - one of: rgb, rbg, gbr, grb, bgr, brg" << endl;
  cerr << "[-o <device>]\toutput device (default: /dev/spidev0.0)" << endl;
  cerr << "<port>" << endl;
}

class HeartbeatSender {
  udp::socket* socket;
  udp::endpoint receiverEndpoint;
  char buf[1];
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

enum RGB_Format{
  RGB,
  RBG,
  BGR,
  BRG,
  GRB,
  GBR
};

RGB_Format parseFormat(const string& s) {
  if(s == "rgb") {
    return RGB;
  } else if(s == "rbg") {
    return RBG;
  } else if(s == "bgr") {
    return BGR;
  } else if(s == "brg") {
    return BRG;
  } else if(s == "grb") {
    return GRB;
  } else if(s == "gbr") {
    return GBR;
  } else {
    printUsage();
    return RGB;
  }
}

int main(int argc, char** argv) {
  try
  {
    int8_t c;
    size_t frameSize = 0;
    string outputFile = "/dev/spidev0.0";
    string dim;
    size_t width = 0;
    size_t height = 0;
    int16_t hue = 0;
    int16_t saturation = 0;
    int16_t lightness = 0;
    RGB_Format pixFormat = RGB;
    bool alternateScanOrder = false;
    while ((c = getopt(argc, argv, "ah:s:l:f:o:d:")) != -1) {
      switch (c) {
      case 'a':
        alternateScanOrder = true;
        break;
      case 'f':
        pixFormat = parseFormat(string(optarg));
        break;
      case 'd':
        dim = string(optarg);
        break;
      case 'o':
        outputFile = optarg;
        break;
      case 'h':
        hue = boost::lexical_cast<int16_t>(optarg);
        break;
      case 's':
        saturation = boost::lexical_cast<int16_t>(optarg);
        break;
      case 'l':
        lightness = boost::lexical_cast<int16_t>(optarg);
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

    size_t port = boost::lexical_cast<size_t>(argv[optind]);
    cerr << "listen: " << port << " ..." << endl;

    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));

    udp::socket::native_type native_sock = socket.native();
    int sendBufferSize = frameSize * 2;
    setsockopt(native_sock, SOL_SOCKET, SO_RCVBUF, &sendBufferSize, sizeof(sendBufferSize));

    char rowBuf[width * 3];
    char recv_buf[frameSize];
    udp::endpoint sender_endpoint;

    for(;;) {
      socket.receive_from(boost::asio::buffer(recv_buf,1), sender_endpoint);

      HeartbeatSender heartbeat(&socket, sender_endpoint);
      boost::thread(&HeartbeatSender::run, heartbeat);

      // don't disable buffering on the spidev
      std::ofstream out(outputFile);
      Fps fps;
      float rate = 1;
      size_t fpsPrintLimit  = 100;

      cerr << "receive from: " << sender_endpoint << endl;
      fps.start();
      bool flipRow = false;

      for(;;) {
        boost::thread receiverThread([&]() {
          socket.receive_from(boost::asio::buffer(recv_buf,frameSize), sender_endpoint);
            for(size_t y = 0; y < height; y++) {
              for(size_t x = 0; x < (width * 3); x+=3) {
                size_t off = y * width * 3;

                RGBPix rgb(recv_buf[off + x], recv_buf[off + x + 1], recv_buf[off + x + 2]);
                HSLPix hsl(rgb);
                hsl.adjustHue(hue);
                hsl.adjustLightness(lightness);
                hsl.adjustSaturation(saturation);

                rgb = RGBPix(hsl);
                size_t one,two,three;

                if(!alternateScanOrder || !flipRow) {
                  one = x;
                  two = x + 1;
                  three = x + 2;
                } else {
                  one = (width*3) - x - 3;
                  two = (width*3) - x - 2;
                  three = (width*3) - x - 1;
                }

                switch(pixFormat) {
                  case RGB:
                    rowBuf[one] = rgb.r;
                    rowBuf[two] = rgb.g;
                    rowBuf[three] = rgb.b;
                  break;
                  case RBG:
                    rowBuf[one] = rgb.r;
                    rowBuf[two] = rgb.b;
                    rowBuf[x + 2] = rgb.g;
                  break;
                  case GRB:
                    rowBuf[one] = rgb.g;
                    rowBuf[two] = rgb.r;
                    rowBuf[three] = rgb.b;
                    break;
                  case GBR:
                    rowBuf[one] = rgb.g;
                    rowBuf[two] = rgb.b;
                    rowBuf[three] = rgb.r;
                    break;
                  case BRG:
                    rowBuf[one] = rgb.b;
                    rowBuf[two] = rgb.r;
                    rowBuf[three] = rgb.g;
                    break;
                  case BGR:
                    rowBuf[one] = rgb.b;
                    rowBuf[two] = rgb.g;
                    rowBuf[three] = rgb.r;
                    break;
                }
              }

              flipRow = !flipRow;
              memcpy(recv_buf + (width * 3 * y), rowBuf, width * 3);
            }
        });

        if(!receiverThread.timed_join(milliseconds(1000))) {
          cerr << endl << "stall client: " << sender_endpoint << endl;
          break;
        }

        out.write(recv_buf, frameSize);
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
