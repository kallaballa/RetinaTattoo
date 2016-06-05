#include "retinacanvas.hpp"

int main() {
  size_t width = 9;
  size_t height = 32;
  size_t frameRate =
      24;
  RetinaCanvas canvas("10.20.30.26", 8888, width, height, frameRate);
  canvas.connect();

  size_t cnt = 0;
  while(true) {
    HSLColor hsl;
    for(size_t y = 0; y < height; ++y) {
      for(size_t x = 0; x < width; ++x) {
        RGBColor rgb(((255 / width) * cnt) + (255 / width) * x,0,0);
        if(rgb.r_ > 255)
          rgb.r_ = rgb.r_ % 255;
        canvas.setPixelRGB(x,y,rgb);
      }
    }
    canvas.update();
    sleep( milliseconds(1000 / frameRate));
    ++cnt;
  }
}
