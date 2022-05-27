
#ifndef PIXEL_HPP
#define PIXEL_HPP

#include <png.h>

namespace im {

   struct pixel {

      png_byte r, g, b;

      pixel() {}
      pixel(png_byte r, png_byte g, png_byte b) : r(r), g(g), b(b) {}
   };
}

#endif

