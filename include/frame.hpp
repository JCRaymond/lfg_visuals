
#ifndef FRAME_HPP
#define FRAME_HPP

#include <pixel.hpp>

#include <functional>
#include <vector>
#include <stdlib.h>
#include <png.h>

namespace im {

   template<size_t width, size_t height>
   struct frame_view;

   template<size_t width, size_t height>
   struct frame {

      pixel* _pixels;
      pixel** _pixel_rows;

      frame() {
         _pixels = new pixel[width * height];
         _pixel_rows = new pixel*[height];
         for (int j = 0; j < height; j++)
            _pixel_rows[j] = &_pixels[j*width];
      }

      frame_view<width, height> view(size_t i, size_t j, size_t n, size_t m) {
         return frame_view<width, height>(this, i, j, n, m);
      }

      template<typename painter>
      inline void paint(painter &p) {
         #pragma omp parallel for schedule(guided)
         for (int j = 0; j < height; j++) {
            int idx = j*width;
            for (int i = 0; i < width; i++) {
               _pixels[idx++] = p.paint(i,j);
            }
         }
      }

      inline pixel paint(unsigned x, unsigned y) {
         return _pixels[y*width + x];
      }

      ~frame() {
         delete[] _pixels;
         delete[] _pixel_rows;
      }
   };

   template<size_t width, size_t height>
   struct frame_view {

      const size_t init_i, init_j;
      const size_t n, m;
      const frame<width, height>* parent;

      frame_view(frame<width, height>* parent, size_t i, size_t j, size_t n, size_t m): parent(parent), init_i(i), init_j(j), n(n), m(m) { }

      frame_view<width, height> view(size_t i, size_t j, size_t n, size_t m) {
         return frame_view<width, height>(parent, init_i + i, init_j + j, n, m);
      }

      template<typename painter>
      inline void paint(painter &p) {
         for (int j = 0; j < m; j++) {
            int idx = (init_j+j)*width + init_i;
            for (int i = 0; i < n; i++) {
               parent->_pixels[idx++] = p.paint(i,j);
            }
         }
      }

      inline pixel paint(unsigned x, unsigned y) {
         return parent->_pixels[(init_j+y)*width + init_i + x];
      }
   };
}

#endif

