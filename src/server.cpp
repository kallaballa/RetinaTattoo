#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/thread.hpp>

#include "heartbeat.hpp"
#include "color.hpp"
#include "fps.hpp"
#include "mapping.hpp"

using namespace boost::posix_time;
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

int main(int argc, char** argv) {
  try
  {
    int8_t c;
    size_t frameSize = 0;
    string outputFile = "/dev/spidev0.0";
    string mappingFile;
    string dim;
    size_t width = 0;
    size_t height = 0;
    int16_t hue = 0;
    int16_t saturation = 0;
    int16_t lightness = 0;
    RGB_Format pixFormat = RGB;
    bool alternateScanOrder = false;
    LedMapping map(0,0,0);

    while ((c = getopt(argc, argv, "ah:s:l:f:o:d:m:")) != -1) {
      switch (c) {
      case 'a':
        alternateScanOrder = true;
        break;
      case 'f':
        pixFormat = parseFormat(string(optarg));
        break;
      case 'm':
        mappingFile = string(optarg);
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

    if(pixFormat == INVALID) {
      std::cerr << "Illegal pixel format!" << std::endl;
      printUsage();
      exit(1);
    }
    if(!dim.empty() && !mappingFile.empty()) {
      std::cerr << "You can't specify both a dimension and a mapping file" << std::endl;
      exit(1);
    }

    boost::char_separator<char> ssep("x", "", boost::keep_empty_tokens);
    if(!dim.empty()) {
      boost::tokenizer<boost::char_separator<char> > tokComponents(dim, ssep);
      auto it = tokComponents.begin();
      width = boost::lexical_cast<size_t>(*it++);
      height = boost::lexical_cast<size_t>(*it);
    } else if(mappingFile.empty()) {
      std::cerr << "You need to  specify either a dimension or a mapping file" << std::endl;
      exit(1);
    } else {
      map = readMappingFile(mappingFile);
      width = map.width();
      height = map.height();
    }

    frameSize = width * height * 3;

    if(argc - optind < 1)
      printUsage();

    size_t port = boost::lexical_cast<size_t>(argv[optind]);
    cerr << "listen: " << port << " ..." << endl;

    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));

    udp::socket::native_type nativeSock = socket.native();
    int sockBufferSize = frameSize * 2;
    setsockopt(nativeSock, SOL_SOCKET, SO_RCVBUF, &sockBufferSize, sizeof(sockBufferSize));


    char* rowBuf = new char[width * 3];
    char* recv_buf = new char[frameSize];

    const size_t frameBufferSize = map.numLeds() * 3;
    char* frameBuffer = new char[frameBufferSize];
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
                  case INVALID:
                    assert(false);
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

        if(map.numLeds() > 0) {
          for(size_t y = 0; y < height; y++) {
            for(size_t x = 0; x < (width * 3); x+=3) {
              size_t off = y * width * 3;

              char& c1 = recv_buf[off + x];
              char& c2 = recv_buf[off + x + 1];
              char& c3 = recv_buf[off + x + 2];

              Coordinate coord;
              coord.x = x / 3;
              coord.y = y;
              std::cerr << coord.x << "/" << coord.y << std::endl;
              assert(map.find(coord) != map.end());
              for(const size_t& pos : map[coord]) {
                frameBuffer[pos * 3] = c1;
                frameBuffer[pos * 3 + 1] = c2;
                frameBuffer[pos * 3 + 2] = c3;
              }
            }
          }
          out.write(frameBuffer, frameBufferSize);
          //flush frame wise
          out.flush();
          memset(frameBuffer, 0, frameBufferSize);
        } else {
          out.write(recv_buf, frameSize);
          //flush frame wise
          out.flush();

        }

        if(fps.next() >= fpsPrintLimit) {
          rate = (fps.sample() + rate) / 2;
          cerr << "\rfps: " << rate;
        }
      }
    }
  }
  catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
