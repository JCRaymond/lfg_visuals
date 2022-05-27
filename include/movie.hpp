
#ifndef MOVIE_HPP
#define MOVIE_HPP

#include <format.hpp>
#include <frame.hpp>
#include <pixel.hpp>

#include <functional>
#include <png.h>
#include <stdlib.h>
#include <string>

#include <iostream>

namespace im {

   template<unsigned width, unsigned height, unsigned fps = 60, unsigned quality = 5>
   struct movie {

      frame<width, height> _frame;
      FILE *_movie;

      movie(std::string filename) {
         std::cout << string_format("ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt rgb%u -s %ux%u -r %u -i - -f mp4 -q:v %u -an -vcodec mpeg4 %s", sizeof(png_byte)*24, width, height, fps, quality, filename) << std::endl;
         _movie = popen(string_format("ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt rgb%u -s %ux%u -r %u -i - -f mp4 -q:v %u -an -vcodec mpeg4 %s", sizeof(png_byte)*24, width, height, fps, quality, filename.c_str()).c_str(), "w");
      }

      inline void write_frame() {
         fwrite(_frame._pixels, sizeof(png_byte), height*width*3, _movie);
      }

      template<typename painter>
      inline void write_frame(painter &p) {
         _frame.paint(p);
         write_frame();
      }

      ~movie() {
         fflush(_movie);
         pclose(_movie);
      }
   };
}

#endif

