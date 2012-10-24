#include "color.h"
#include <iostream>

  RGB::RGB(const uint8_t& r, const uint8_t& g, const uint8_t& b) :
    r(r),
    g(g),
    b(b) {
  }

  RGB::RGB(const HSL& hsl) :
      r(0), g(0), b(0) {
    float L = ((float) hsl.l) / 100;
    float S = ((float) hsl.s) / 100;
    float H = ((float) hsl.h) / 360;

    if (hsl.s == 0) {
      r = hsl.l;
      g = hsl.l;
      b = hsl.l;
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
          HSLtoRGB_Subfunction(r, temp1, temp2, temp3);
          break;
        }
        case 1: // green
        {
          temp3 = H;
          HSLtoRGB_Subfunction(g, temp1, temp2, temp3);
          break;
        }
        case 2: // blue
        {
          temp3 = H - .33333f;
          if (temp3 < 0)
            temp3 += 1;
          HSLtoRGB_Subfunction(b, temp1, temp2, temp3);
          break;
        }
        }
      }
    }
    r = (unsigned int) ((((float) r) / 100) * 255);
    g = (unsigned int) ((((float) g) / 100) * 255);
    b = (unsigned int) ((((float) b) / 100) * 255);
  }

  RGB::RGB(const uint32_t& v) :
    r((uint8_t)((v>>16)&0xFF)),
    g((uint8_t)((v>>8)&0xFF)),
    b((uint8_t)(v&0xFF)){
  }

  // This is a subfunction of HSLtoRGB
  void RGB::HSLtoRGB_Subfunction(uint8_t& c, const float& temp1, const float& temp2,
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

  const uint32_t RGB::val() const {
    return ((uint32_t)r<<16) | ((uint32_t)g<<8) | b;
  }


  HSL::HSL(const RGB& rgb) {
    float r_percent = ((float) rgb.r) / 255;
    float g_percent = ((float) rgb.g) / 255;
    float b_percent = ((float) rgb.b) / 255;

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
    s = (unsigned int) (S * 100);
    l = (unsigned int) (L * 100);
    H = H * 60;
    if (H < 0)
      H += 360;
    h = (unsigned int) H;
  }

  void HSL::adjustHue(const int16_t& amount) {
    h+=amount;

    if (h < 0) {
      h = h % -360;
    } else if ( h > 360 ) {
      h = h % 360;
    }
  }

  void HSL::adjustSaturation(const int16_t& amount) {
    s+=amount;
    if (s < 0) {
      s = s * -1;
    }

    if ( s > 100 ) {
      s = 100;
    }
  }


  void HSL::adjustLightness(const int16_t& amount) {
    l+=amount;

    if (l < 0) {
      l = l * -1;
    }

    if ( l > 100 ) {
      l = 100;
    }
  }
