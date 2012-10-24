#include <stdint.h>

#ifndef HSL2RGB_H
#define HSL2RGB_H

class HSL;
struct RGB {
  uint8_t r,g,b;
  RGB(const uint8_t& r, const uint8_t& g, const uint8_t& b);
  RGB(const HSL& hsl);
  RGB(const uint32_t& v);
  void HSLtoRGB_Subfunction(uint8_t& c, const float& temp1, const float& temp2, const float& temp3);
  const uint32_t val() const;
};

struct HSL {
  int16_t h,s,l;
  HSL(const RGB& rgb);
  void adjustHue(const int16_t& amount);
  void adjustSaturation(const int16_t& amount);
  void adjustLightness(const int16_t& amount);
};

#endif
