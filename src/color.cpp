#include "color.hpp"
#include <iostream>

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
    return INVALID;
  }
}

RGBColor::RGBColor(const int16_t& r, const int16_t& g, const int16_t& b) :
  r_(r),
  g_(g),
  b_(b) {
}

RGBColor::RGBColor(const HSLColor& hsl) :
    r_(0), g_(0), b_(0) {
  float L = ((float) hsl.l_) / 100;
  float S = ((float) hsl.s_) / 100;
  float H = ((float) hsl.h_) / 360;

  if (hsl.s_ == 0) {
    r_ = hsl.l_;
    g_ = hsl.l_;
    b_ = hsl.l_;
  } else {
    float temp1 = 0;
    if (L < .50) {
      temp1 = L * (1 + S);
    } else {
      temp1 = L + S - (L * S);
    }

    float temp2 = 2 * L - temp1;

    float temp3 = 0;
    for (int i = 0; i < 3; i++) {
      switch (i) {
      case 0: // red
      {
        temp3 = H + .33333f;
        if (temp3 > 1)
          temp3 -= 1;
        HSLtoRGB_Subfunction(r_, temp1, temp2, temp3);
        break;
      }
      case 1: // green
      {
        temp3 = H;
        HSLtoRGB_Subfunction(g_, temp1, temp2, temp3);
        break;
      }
      case 2: // blue
      {
        temp3 = H - .33333f;
        if (temp3 < 0)
          temp3 += 1;
        HSLtoRGB_Subfunction(b_, temp1, temp2, temp3);
        break;
      }
      }
    }
  }
  r_ = (unsigned int) ((((float) r_) / 100) * 255);
  g_ = (unsigned int) ((((float) g_) / 100) * 255);
  b_ = (unsigned int) ((((float) b_) / 100) * 255);
}

RGBColor::RGBColor(const uint32_t& v) :
  r_((uint8_t)((v>>16)&0xFF)),
  g_((uint8_t)((v>>8)&0xFF)),
  b_((uint8_t)(v&0xFF)){
}

// This is a subfunction of HSLtoRGB
void RGBColor::HSLtoRGB_Subfunction(int16_t& c, const float& temp1, const float& temp2,
    const float& temp3) {
  if ((temp3 * 6) < 1)
    c = (unsigned int) ((temp2 + (temp1 - temp2) * 6 * temp3) * 100);
  else if ((temp3 * 2) < 1)
    c = (unsigned int) (temp1 * 100);
  else if ((temp3 * 3) < 2)
    c =
        (unsigned int) ((temp2 + (temp1 - temp2) * (.66666 - temp3) * 6) * 100);
  else
    c = (unsigned int) (temp2 * 100);
  return;
}

const uint32_t RGBColor::val() const {
  return ((uint32_t)r_<<16) | ((uint32_t)g_<<8) | b_;
}


HSLColor::HSLColor(const RGBColor& rgb) {
  float r_percent = ((float) rgb.r_) / 255;
  float g_percent = ((float) rgb.g_) / 255;
  float b_percent = ((float) rgb.b_) / 255;

  float max_color = 0;
  if ((r_percent >= g_percent) && (r_percent >= b_percent)) {
    max_color = r_percent;
  }
  if ((g_percent >= r_percent) && (g_percent >= b_percent))
    max_color = g_percent;
  if ((b_percent >= r_percent) && (b_percent >= g_percent))
    max_color = b_percent;

  float min_color = 0;
  if ((r_percent <= g_percent) && (r_percent <= b_percent))
    min_color = r_percent;
  if ((g_percent <= r_percent) && (g_percent <= b_percent))
    min_color = g_percent;
  if ((b_percent <= r_percent) && (b_percent <= g_percent))
    min_color = b_percent;

  float L = 0;
  float S = 0;
  float H = 0;

  L = (max_color + min_color) / 2;

  if (max_color == min_color) {
    S = 0;
    H = 0;
  } else {
    if (L < .50) {
      S = (max_color - min_color) / (max_color + min_color);
    } else {
      S = (max_color - min_color) / (2 - max_color - min_color);
    }
    if (max_color == r_percent) {
      H = (g_percent - b_percent) / (max_color - min_color);
    }
    if (max_color == g_percent) {
      H = 2 + (b_percent - r_percent) / (max_color - min_color);
    }
    if (max_color == b_percent) {
      H = 4 + (r_percent - g_percent) / (max_color - min_color);
    }
  }
  s_ = (unsigned int) (S * 100);
  l_ = (unsigned int) (L * 100);
  H = H * 60;
  if (H < 0)
    H += 360;
  h_ = (unsigned int) H;
}

void HSLColor::adjustHue(const int16_t& amount) {
  h_+=amount;

  if (h_ < 0) {
    h_ = h_ % -360;
  } else if ( h_ > 360 ) {
    h_ = h_ % 360;
  }
}

void HSLColor::adjustSaturation(const int16_t& amount) {
  s_+=amount;
  if (s_ < 0) {
    s_ = s_ * -1;
  }

  if ( s_ > 100 ) {
    s_ = 100;
  }
}


void HSLColor::adjustLightness(const int16_t& amount) {
  l_+=amount;

  if (l_ < 0) {
    l_ = l_ * -1;
  }

  if ( l_ > 100 ) {
    l_ = 100;
  }
}
