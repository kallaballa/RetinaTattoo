#include <ext/stdio_filebuf.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
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
//#include "color.h"
#include <boost/tokenizer.hpp>

using namespace boost::posix_time;
using namespace boost::this_thread;
using boost::asio::ip::udp;
using std::string;
using std::cerr;
using std::endl;

unsigned char *convert(unsigned char *input, int w)
{
  unsigned char *bitmap = new unsigned char[w*10/8];
  int bitcounter = 0;
  unsigned char tmp = 0;
  int outindex = 0;

  std::cerr << ">>>>>>>>>>>" << std::endl;
  for (int x=0;x<w;x++) {
    std::cerr << "###" << std::endl;
    for (int16_t y=-1;y<9;y++) {
      unsigned char on = (y >= 0 && input[y*w+x] > 128) ? 1 : 0;
      size_t val = ((pow(2,7 - bitcounter)) * on);
      tmp += val;
      std::cerr << "bit:" << bitcounter << " pix:" << (int) on << " tmp: " << (int) tmp << " val: " << (int) val << std::endl;
      bitcounter++;

      if (bitcounter == 8) {
        bitmap[outindex++] = tmp;
        bitcounter = 0;
        tmp = 0;
      //  std::cerr << (int)input[y*w+x] << " -> " << (int)bitmap[outindex - 1] << std::endl;
      }
    }
    std::cerr << "###" << std::endl;
  }

  std::cerr << "<<<<<<<<<<<" << std::endl;

  return bitmap;
}


unsigned char *convert_(unsigned char *input, int w)
{
  unsigned char *bitmap = new unsigned char[w*10/8];
  int bitcounter = 0;
  unsigned char tmp = 0;
  int outindex = 0;
  for (int x=0;x<w;x++) {
    tmp = 0b10000000;
    ++bitcounter;

    for (int y=0;y<10;y++) {
      unsigned char pixel = input[y*w+x] > 128;
      tmp |= (pixel >> bitcounter++);
      std::cerr << "pix:" << (int) pixel << " tmp: " << (int) tmp << std::endl;
      if (bitcounter == 8) {
        bitmap[outindex++] = tmp;
        bitcounter = 0;
        tmp = 0;
        std::cerr << (int)input[y*w+x] << " -> " << (int)bitmap[outindex - 1] << std::endl;
      }
    }
  }

  return bitmap;
}

void printUsage() {
  cerr
      << "Usage: server [-a][-o<output device>][-d<dimension>][-h<hue>][-s<saturation>][-l<lightness>][-f <pixFormat>] <port>"
      << endl;
  cerr
      << "[-h <hue>]\t hue modifier either an offset in degrees or a floating point scale"
      << endl;
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
      socket(socket), receiverEndpoint(receiverEndpoint) {
    buf[0] = 0;
  }
  void run() {
    for (;;) {
      socket->send_to(boost::asio::buffer(buf), receiverEndpoint);
      boost::this_thread::sleep(milliseconds(300));
    }
  }
};

int main(int argc, char** argv) {
  try {
    int8_t c;
    string outputFile = "/dev/spidev0.0";
    string dim;
    size_t width = 0;
    size_t height = 0;
    int16_t hue = 0;
    int16_t saturation = 0;
    int16_t lightness = 0;
    bool alternateScanOrder = false;
    while ((c = getopt(argc, argv, "ah:s:l:f:o:d:")) != -1) {
      switch (c) {
      case 'a':
        alternateScanOrder = true;
        break;
      case 'f':
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
    const size_t frameSize = width * height;

    if (argc - optind < 1)
      printUsage();

    size_t port = boost::lexical_cast<size_t>(argv[optind]);
    cerr << "listen: " << port << " ..." << endl;

    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));

    udp::socket::native_type native_sock = socket.native();
    int sendBufferSize = frameSize * 2;
    setsockopt(native_sock, SOL_SOCKET, SO_RCVBUF, &sendBufferSize,
        sizeof(sendBufferSize));

    unsigned char recv_buf[frameSize];
    udp::endpoint sender_endpoint;
    int ret = 0;
    int speed = 100000;
    int fd = open(outputFile.data(), O_WRONLY);
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    __gnu_cxx ::stdio_filebuf<char> filebuf(fd, std::ios::out);
    std::ostream out(&filebuf);

    unsigned char *bitmap;
    for (;;) {
      socket.receive_from(boost::asio::buffer(recv_buf, 1), sender_endpoint);

      HeartbeatSender heartbeat(&socket, sender_endpoint);
      boost::thread(&HeartbeatSender::run, heartbeat);

      Fps fps;
      float rate = 1;
      size_t fpsPrintLimit = 100;

      cerr << "receive from: " << sender_endpoint << endl;
      fps.start();

      for (;;) {
        socket.receive_from(boost::asio::buffer(recv_buf,frameSize), sender_endpoint);

        for(size_t i = 0; i < (width*height); ++i) {
          std::cerr << (int)recv_buf[i] << " ";
        }

        std::cerr << endl << endl;

        //FIXME delete
        bitmap = convert(recv_buf, width);

        for(size_t i = 0; i < (width*10/8); ++i) {
          out << bitmap[i];
          std::cout << bitmap[i];
        }

        //flush frame wise
        out.flush();
        std::cout.flush();

        if (fps.next() >= fpsPrintLimit) {
          rate = (fps.sample() + rate) / 2;
          cerr << "\rfps: " << rate;
        }
      }
    }
  } catch (std::exception& e) {
    std::cerr <<  "Exception: " << e.what() << std::endl;
  }
  return 0;
}

