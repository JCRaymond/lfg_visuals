#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <pixel.hpp>
#include <frame.hpp>

#include <functional>
#include <stdlib.h>
#include <png.h>

#include <iostream>

namespace im {

   template<unsigned width, unsigned height>
   struct image {
      
   private:

      struct read_painter {
         png_bytepp rows;

         read_painter(png_bytepp rows) : rows(rows) {}

         inline pixel paint(unsigned x, unsigned y) {
            return ((pixel**)rows)[y][x];
         }
      };

   public:

      im::frame<width, height> _image;

      template<typename painter>
      inline void paint_frame(painter &p) {
         _image.paint(p);
      }

      inline pixel paint(unsigned x, unsigned y) {
         return _image.paint(x,y);
      }

      int read(const char* fname) {
         FILE* f = fopen(fname, "rb");
         if (!f) {
            return 1;
         }

         char header[8];
         int i = fread(header, 1, 8, f);
         if (i != 8)
            return 2;
         if (png_sig_cmp((png_bytep)header, 0, 8))
            return 3;

         png_structp png_ptr;
         png_infop info_ptr;

         png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
         if (!png_ptr) {
            return 4;
         }

         info_ptr = png_create_info_struct(png_ptr);
         if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
            return 5;
         }

         png_init_io(png_ptr, f);
         png_set_sig_bytes(png_ptr, 8);
         png_read_info(png_ptr, info_ptr);

         unsigned color_type = png_get_color_type(png_ptr, info_ptr);
         unsigned bit_depth = png_get_bit_depth(png_ptr, info_ptr);
         if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
            png_set_expand(png_ptr);
         if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand(png_ptr);
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);
         if (bit_depth == 16)
            png_set_strip_16(png_ptr);
         if (color_type & PNG_COLOR_MASK_ALPHA)
            png_set_strip_alpha(png_ptr);

         unsigned f_width = png_get_image_width(png_ptr, info_ptr);
         if (f_width != width)
            return 6;
         unsigned f_height = png_get_image_height(png_ptr, info_ptr);
         if (f_height != height)
            return 7;

         png_read_image(png_ptr, (png_bytepp)_image._pixel_rows);

         png_read_end(png_ptr, NULL);
         png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

         fclose(f);
         f = NULL;

         return 0;
      }

      int write(const char* fname) {
         png_structp png_ptr;
         png_infop info_ptr;

         png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
         if (!png_ptr) {
            return 1;
         }

         info_ptr = png_create_info_struct(png_ptr);
         if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            return 2;
         }

         FILE* f = fopen(fname, "wb");
         if (!f) {
            return 3;
         }

         png_init_io(png_ptr, f);
         png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
         png_write_info(png_ptr, info_ptr);
         
         png_write_image(png_ptr, (png_bytepp)_image._pixel_rows);
         png_write_end(png_ptr, NULL);
         png_destroy_write_struct(&png_ptr, &info_ptr);

         fclose(f);
         f = NULL;

         return 0;
      }
   };
}

#endif

