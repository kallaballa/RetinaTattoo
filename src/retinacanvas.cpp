#include "retinacanvas.hpp"
#include <boost/thread.hpp>

using boost::this_thread::sleep;
using boost::posix_time::milliseconds;

using std::cerr;
using std::endl;

RetinaCanvas::RetinaCanvas(string host, uint16_t port, size_t width, size_t height, size_t frameRate) :
    host_(host),
    port_(port),
    width_(width),
    height_(height),
    frameSize_(width * height * 3),
    frameRate_(frameRate),
    fps_(),
    io_service_(),
    socket_(io_service_),
    hearbeat_(&socket_),
    frameBuffer_(frameSize_),
    rowBuffer_(width_ * 3),
    endpoint_(),
    frameBufferMutex_(),
    update_(false) {
  }

void RetinaCanvas::connect() {
  cerr << "connecting: " << host_ << ":" << port_ << endl;

  //prepare the udp socket
  socket_.open(udp::v4());

  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(),host_, std::to_string(port_));
  endpoint_ = *resolver.resolve(query);

  //set send buffer size to 2 frames
  udp::socket::native_type native_sock = socket_.native();
  int buffsize = frameSize_ * 2;
  setsockopt(native_sock, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize));

  boost::thread(&HeartbeatReceiver::run, &hearbeat_);

  boost::system::error_code ignored_error;

  fps_.start();

  cerr << "waiting..." << endl;
  char hb[1] = { 0 };
  while (!hearbeat_.alive()) {
    socket_.send_to(boost::asio::buffer(hb, 1),endpoint_, 0, ignored_error);
    sleep( milliseconds(1000) );
  }
  cerr << "waiting done..." << endl;
}

void RetinaCanvas::setPixelRGB(const size_t& x, const size_t& y, const RGBColor& rgb) {
  frameBufferMutex_.lock();
  size_t off = y * width_ * 3;

  frameBuffer_[off + (x * 3)] = rgb.r_;
  frameBuffer_[off + (x * 3) + 1] = rgb.g_;
  frameBuffer_[off + (x * 3) + 2] = rgb.b_;

  frameBufferMutex_.unlock();
}

void RetinaCanvas::update() {
  frameBufferMutex_.lock();

  if (hearbeat_.alive()) {
    boost::system::error_code ignored_error;

    bool forward = true;
    std::cerr << "sending buffer" << std::endl;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < (width_ * 3); x += 3) {
        size_t off = y * width_ * 3;

        if (!forward) {
          rowBuffer_[x] = frameBuffer_[off + x];
          rowBuffer_[x + 1] = frameBuffer_[off + x + 1];
          rowBuffer_[x + 2] = frameBuffer_[off + x + 2];
        } else {
          rowBuffer_[(width_ * 3) - x - 3] = frameBuffer_[off + x];
          rowBuffer_[(width_ * 3) - x - 2] = frameBuffer_[off + x + 1];
          rowBuffer_[(width_ * 3) - x - 1] = frameBuffer_[off + x + 2];
        }
      }

      forward = !forward;
      memcpy(frameBuffer_.data() + (width_ * 3 * y), rowBuffer_.data(), width_ * 3);
    }

    socket_.send_to(asio::buffer(frameBuffer_.data(), frameSize_), endpoint_, 0, ignored_error);
  }

  size_t fpsPrintLimit = 100;
  float targetDur = (1000.0 / frameRate_);
  float rate = 1;
  if (fps_.next() >= fpsPrintLimit) {
    float sampledRate = fps_.sample();
    rate = (sampledRate + rate) / 2;
    cerr << "\rfps: " << rate;
  }

  sleep(milliseconds(targetDur));

  frameBufferMutex_.unlock();
}

