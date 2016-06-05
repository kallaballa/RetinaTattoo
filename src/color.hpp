#ifndef HSL2RGB_H
#define HSL2RGB_H

#include <stdint.h>
#include <string>

using std::string;

enum RGB_Format{
  RGB,
  RBG,
  BGR,
  BRG,
  GRB,
  GBR,
  INVALID
};

RGB_Format parseFormat(const string& s);

class HSLColor;
class RGBColor {
  void HSLtoRGB_Subfunction(int16_t& c, const float& temp1, const float& temp2, const float& temp3);
public:
  int16_t r_,g_,b_;
  RGBColor(const int16_t& r, const int16_t& g, const int16_t& b);
  RGBColor(const HSLColor& hsl);
  RGBColor(const uint32_t& v);

  const uint32_t val() const;
};

class HSLColor {
public:
  int16_t h_,s_,l_;
  HSLColor() : h_(0), s_(0), l_(0) {

  }
  HSLColor(const RGBColor& rgb);
  void adjustHue(const int16_t& amount);
  void adjustSaturation(const int16_t& amount);
  void adjustLightness(const int16_t& amount);
};

#endif
