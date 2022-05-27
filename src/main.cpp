#include <iostream>
#include <cmath>

#include <image.hpp>

#include <stdint.h>

/* This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
   It has excellent (sub-ns) speed, a state (256 bits) that is large
   enough for any parallel application, and it passes all tests we are
   aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


static uint64_t s[4] = {684684,6546843,219681,468984};

uint64_t next(void) {
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

template <size_t N, size_t ITERS=8>
void next_perm(size_t (&p)[N]) {
   for (int i=0; i<N; i++)
      p[i] = i;
   for (int iter=0; iter<ITERS; iter++) {
      size_t a, b;
      a = next() % N;
      do {
         b = next() % N;
      } while (b == a);
      std::swap(p[a], p[b]);
   }
}

template <size_t D, size_t N>
void next_color(const im::pixel (&colors)[D], im::pixel (&c)[N]) {
   for (int i=0; i<N; i++)
      c[i] = colors[next() % D];
}

template<size_t w, size_t l, size_t t=3>
struct bg_painter {
   im::pixel paint(size_t x, size_t y) {
      if (x < t or y < t or x >= (w-t) or y >= (l-t))
         return {64, 64, 64};
      return {255, 255, 255};
   }
};

template <size_t lw, size_t sep, size_t sl, size_t bw=3>
im::image<2*lw + 3*sep, 2*lw + 3*sep> piece(size_t (&p)[4], im::pixel (&c)[4]) {
   static const int n = 2*lw + 3*sep;
   static auto bg = bg_painter<n, n, bw>();
   size_t p_inv[4];
   for (int d=0; d<4; d++)
      p_inv[p[d]] = d;
   im::image<n, n> pi;
   pi.paint_frame(bg);
   int tmp;
   int dx = 1;
   int dy = 1;
   int ix = sep;
   int iy = 0;
   int tlw = lw;
   int tsl = sl;
   for (int d=0; d<4; d++) {

      for (int x=0; x<tlw; x++) {
         for (int y=0; y<tsl; y++) {
            pi._image._pixel_rows[iy + y*dy][ix + x*dx] = c[d];
         }
      }

      tmp = dy;
      dy = dx;
      dx = -tmp;

      tmp = iy;
      iy = ix;
      ix = n - 1 - tmp;

      std::swap(tlw, tsl);
   }

   ix = n - sep - lw;
   int bx = lw;
   int by = 0;
   for (int d=0; d<4; d++) {
      for (int i=1; i<bx; i++) {
         for (int j=0; j<i; j++) {
            pi._image._pixel_rows[iy + i*dy][ix - (j+1)*dx] = c[p_inv[d]];
            pi._image._pixel_rows[iy + i*dy][ix + (lw + j)*dx] = c[p_inv[d]];
         }
      }
      for (int i=1; i<by; i++) {
         for (int j=0; j<i; j++) {
            pi._image._pixel_rows[iy - (j+1)*dy][ix + i*dx] = c[p_inv[d]];
            pi._image._pixel_rows[iy + (lw + j)*dy][ix + i*dx] = c[p_inv[d]];
         }
      }
      std::swap(bx, by);

      for (int x=0; x<tlw; x++) {
         for (int y=0; y<tsl; y++) {
            pi._image._pixel_rows[iy + y*dy][ix + x*dx] = c[p_inv[d]];
         }
      }

      tmp = dy;
      dy = dx;
      dx = -tmp;

      tmp = iy;
      iy = ix;
      ix = n - 1 - tmp;

      std::swap(tlw, tsl);
   }
   
   ix = sep;
   iy = sl;
   dx = 1;
   dy = 1;
   for (int d=0; d<4; d++) {
      int o = p[d];
      int rel = (o - d + 4) % 4;
      int sx, sy, steps;
      switch (rel) {
         case 0: {
            sx = 1;
            sy = 0;
            steps = sep + lw;
            break;
         }
         case 1: {
            sx = 1;
            sy = 1;
            steps = 2*sep + lw - sl;
            break;
         }
         case 2: {
            sx = 0;
            sy = 1;
            steps = 3*sep - 2*sl + lw;
            break;
         }
         case 3: {
            sx = -1;
            sy = 1;
            steps = sep - sl;
            break;
         }
      }
      for (int t=0; t<d; t++) {
         tmp = sy;
         sy = sx;
         sx = -tmp;
      }
      int w = lw;
      if (sx != 0 and sy !=0) {
         w = ((int) (((double)lw)/sqrt(2))) + 1;
         if (d % 2 == 0) {
            iy -= dy*(lw-w);
            if (rel == 3)
               ix += dx*(lw-w);
         }
         else {
            ix -= dx*(lw-w);
            if (rel == 3)
               iy += dy*(lw-w);
         }
         steps += 2*(lw-w);
      }
      for (int i=0; i<steps+1; i++) {
         for (int x=0; x<w; x++) {
            for (int y=0; y<w; y++) {
               pi._image._pixel_rows[iy + i*sy + y*dy][ix + i*sx + x*dx] = c[d];
            }
         }
      }
      if (sx != 0 and sy !=0) {
         if (d % 2 == 0) {
            iy += dy*(lw-w);
            if (rel == 3)
               ix -= dx*(lw-w);
         }
         else {
            ix += dx*(lw-w);
            if (rel == 3)
               iy -= dy*(lw-w);
         }
      }
      tmp = dy;
      dy = dx;
      dx = -tmp;

      tmp = iy;
      iy = ix;
      ix = n - 1 - tmp;
   }
   return pi;
}

void piece_main() {
   const size_t lw = 30;
   const size_t sep = 100;
   const size_t sl = 50;

   size_t p[4] = {1, 2, 0, 3};
   im::pixel c[4] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}};
   piece<lw, sep, sl>(p, c).write("piece4.png");
}

void rand_main() {
   const size_t lw = 30;
   const size_t sep = 100;
   const size_t sl = 50;
   const size_t ps = 3*sep + 2*lw;
   const size_t n = 10;
   const size_t m = 10;
   const size_t w = n*ps;
   const size_t h = m*ps;

   im::image<w, h> pattern;

   size_t pieces[n][m][4];
   im::pixel colors[n][m][4];

   size_t p[4] = {0, 1, 2, 3};
   int is[4] = {0, 1, 2, 3};
   int t1 = 1;
   int t2 = 0;
   int t3 = 3;
   im::pixel rgb[3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         std::swap(p[is[0]], p[is[1]]);
         std::swap(p[is[2]], p[is[3]]);
         is[0] = (is[0] + t1)%4;
         is[1] = (is[1] + t1)%4;
         is[2] = (is[2] + t3)%4;
         is[3] = (is[4] + t3)%4;
         if ((i+j+is[2])%5 == 1 or (i+j+is[2])%5 == 3)
            t1 = (t1 + 2)%4;
         if ((i+j)%3 >= 1)
            t3 = (t3 + 2)%4;
         for (int k=0; k<4; k++) {
            pieces[i][j][k] = p[k];
            colors[i][j][k] = {0, 0, 0};
            /*
            if (is[k] != 3)
               colors[i][j][k] = rgb[is[k]];
            else {
               colors[i][j][k] = rgb[t2];
               t2 = (t2 + 1)%3;
            }
            */
         }
      }
   }
   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         auto pattern_piece = piece<lw, sep, sl>(pieces[i][j], colors[i][j]);
         pattern._image.view(i*ps, j*ps, ps, ps).paint(pattern_piece);
      }
   }

   pattern.write("rand_pattern_bw.png");
}

void path_main() {
   const size_t lw = 30;
   const size_t sep = 100;
   const size_t sl = 50;
   const size_t ps = 3*sep + 2*lw;
   const size_t n = 10;
   const size_t m = 10;
   const size_t w = n*ps;
   const size_t h = m*ps;

   im::image<w, h> pattern;
   size_t a = 8;
   size_t b = 7;
   size_t i = 0;

   size_t p[4] = {0, 1, 2, 3};
   im::pixel c[4] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
   im::pixel bc[4] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         auto pattern_piece = piece<lw, sep, sl>(p, bc);
         pattern._image.view(i*ps, j*ps, ps, ps).paint(pattern_piece);
      }
   }
   pattern.write("loop_0_bw.png");

   c[0] = {255, 0, 0};
   auto pattern_piece = piece<lw, sep, sl>(p, bc);
   pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);
   c[0] = {0, 0, 0};
   b -= 1;


   c[2] = {255, 0, 0};
   pattern_piece = piece<lw, sep, sl>(p, bc);
   pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);
   c[2] = {0, 0, 0};
   pattern.write("loop_1_bw.png");

   for (int i=0; i<4; i++) {
      p[0] = 0;
      p[1] = 1;
      p[2] = 3;
      p[3] = 2;
      c[0] = {0, 0, 0};
      c[1] = {0, 0, 0};
      c[2] = {0, 255, 0};
      c[3] = {0, 0, 255};
      pattern_piece = piece<lw, sep, sl>(p, bc);
      pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);

      a -= 1;
      p[0] = 0;
      p[1] = 3;
      p[2] = 2;
      p[3] = 1;
      c[0] = {0, 0, 0};
      c[1] = {0, 0, 255};
      c[2] = {0, 0, 0};
      c[3] = {0, 255, 0};
      pattern_piece = piece<lw, sep, sl>(p, bc);
      pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);

      a -= 1;
      p[0] = 1;
      p[1] = 0;
      p[2] = 2;
      p[3] = 3;
      c[0] = {255, 0, 0};
      c[1] = {255, 0, 0};
      c[2] = {0, 0, 0};
      c[3] = {0, 0, 0};
      pattern_piece = piece<lw, sep, sl>(p, bc);
      pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);
      b -= 1;
   }

   b += 1;
   p[0] = 0;
   p[1] = 1;
   p[2] = 2;
   p[3] = 3;
   c[0] = {0, 0, 0};
   c[1] = {255, 0, 0};
   c[2] = {0, 0, 0};
   c[3] = {0, 0, 0};
   pattern_piece = piece<lw, sep, sl>(p, bc);
   pattern._image.view(a*ps, b*ps, ps, ps).paint(pattern_piece);

   pattern.write("loop_5_bw.png");

}


template <int N>
void inv_perm(const size_t (&p1)[N], size_t (&p2)[N]) {
    for (int i=0; i<N; i++)
        p2[p1[i]] = i;
}

im::pixel rot_color(im::pixel color) {
    im::pixel ret = color;
    std::swap(ret.r, ret.g);
    std::swap(ret.r, ret.b);
    return ret;
}

int sft_main() {
   const size_t lw = 2;
   const size_t sep = 5;
   const size_t sl = 2;
   const size_t bw = 0;
   const size_t ps = 3*sep + 2*lw;
   const size_t n = 300;
   const size_t m = 300;
   const size_t w = n*ps;
   const size_t h = m*ps;

   im::image<w, h> pattern;

   const size_t D = 3;
   im::pixel colors[D] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};

   size_t pieces[n][m][4];
   size_t inv_pieces[n][m][4];
   im::pixel pcolors[n][m][4];
   
   next_perm<4>(pieces[0][0]);
   inv_perm<4>(pieces[0][0], inv_pieces[0][0]);
   next_color<D, 4>(colors, pcolors[0][0]);
   //auto pattern_piece = piece<lw, sep, sl>(pieces[0][0], pcolors[0][0]);
   //pattern._image.view(0, 0, ps, ps).paint(pattern_piece);

   // top row
   for (int i=1; i<n; i++) {
      do {
         next_perm<4>(pieces[i][0]);
         inv_perm<4>(pieces[i][0], inv_pieces[i][0]);
      } while (pieces[i][0][3] == 3 and pieces[i-1][0][1] != 1 and pcolors[i-1][0][1] != rot_color(rot_color(pcolors[i-1][0][inv_pieces[i-1][0][1]])));
      next_color<D, 4>(colors, pcolors[i][0]);
      pcolors[i][0][3] = rot_color(pcolors[i-1][0][inv_pieces[i-1][0][1]]);
      pcolors[i][0][inv_pieces[i][0][3]] = rot_color(rot_color(pcolors[i-1][0][1]));

      //auto pattern_piece = piece<lw, sep, sl>(pieces[i][0], pcolors[i][0]);
      //pattern._image.view(i*ps, 0, ps, ps).paint(pattern_piece);
   }

   // left col
   for (int j=1; j<m; j++) {
      do {
         next_perm<4>(pieces[0][j]);
         inv_perm<4>(pieces[0][j], inv_pieces[0][j]);
      } while (pieces[0][j][0] == 0 and pieces[0][j-1][2] != 2 and pcolors[0][j-1][2] != rot_color(rot_color(pcolors[0][j-1][inv_pieces[0][j-1][2]])));
      next_color<D, 4>(colors, pcolors[0][j]);
      pcolors[0][j][0] = rot_color(pcolors[0][j-1][inv_pieces[0][j-1][2]]);
      pcolors[0][j][inv_pieces[0][j][0]] = rot_color(rot_color(pcolors[0][j-1][2]));

      //auto pattern_piece = piece<lw, sep, sl>(pieces[0][j], pcolors[0][j]);
      //pattern._image.view(0, j*ps, ps, ps).paint(pattern_piece);
   }

   // rest
   for (int i=1; i<n; i++) {
      for (int j=1; j<m; j++) {
         do {
            next_perm<4>(pieces[i][j]);
            inv_perm<4>(pieces[i][j], inv_pieces[i][j]);
         } while (not(
            (
                (pieces[i][j][0] != 0 and pieces[i][j][0] != 3) or
                (pieces[i][j][0] == 0 and pieces[i][j-1][2] == 2) or
                (pieces[i][j][0] == 0 and pcolors[i][j-1][2] == rot_color(rot_color(pcolors[i][j-1][inv_pieces[i][j-1][2]]))) or
                (pieces[i][j][0] == 3 and pcolors[i-1][j][1] == rot_color(rot_color(pcolors[i][j-1][inv_pieces[i][j-1][2]])))
            )
            and
            (
                (pieces[i][j][3] != 0 and pieces[i][j][3] != 3) or
                (pieces[i][j][3] == 3 and pieces[i-1][j][1] == 1) or
                (pieces[i][j][3] == 3 and pcolors[i-1][j][1] == rot_color(rot_color(pcolors[i-1][j][inv_pieces[i-1][j][1]]))) or
                (pieces[i][j][3] == 0 and pcolors[i][j-1][2] == rot_color(rot_color(pcolors[i-1][j][inv_pieces[i-1][j][1]])))
            )
         ));
         next_color<D, 4>(colors, pcolors[i][j]);
         pcolors[i][j][0] = rot_color(pcolors[i][j-1][inv_pieces[i][j-1][2]]);
         pcolors[i][j][inv_pieces[i][j][0]] = rot_color(rot_color(pcolors[i][j-1][2]));
         pcolors[i][j][3] = rot_color(pcolors[i-1][j][inv_pieces[i-1][j][1]]);
         pcolors[i][j][inv_pieces[i][j][3]] = rot_color(rot_color(pcolors[i-1][j][1]));

         //auto pattern_piece = piece<lw, sep, sl>(pieces[i][j], pcolors[i][j]);
         //pattern._image.view(i*ps, j*ps, ps, ps).paint(pattern_piece);
         //pattern.write("test.png");
      }
   }

   /*
   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         auto pattern_piece = piece<lw, sep, sl, bw>(pieces[i][j], pcolors[i][j]);
         pattern._image.view(i*ps, j*ps, ps, ps).paint(pattern_piece);
      }
   }
   pattern.write("test.png");
   */

   // Shuffling
   std::cout << "shuffling" << std::endl;
   const int pn = 3;
   const int pd = 5;
   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         for (int x=0; x<3; x++) {
            for (int y=x+1; y<4; y++) {
               if (pcolors[i][j][x] == pcolors[i][j][y]) {
                  if (pieces[i][j][x] == x) {
                     int dx = -1 + 2*(x/2);
                     int xop = (x + 2) % 4;
                     if (x%2==0 and (0 <= j+dx) and (j+dx < m) and pieces[i][j+dx][xop] == xop)
                        goto nextx;
                     if (x%2==1 and (0 <= i-dx) and (i-dx < n) and pieces[i-dx][j][xop] == xop)
                        goto nextx;
                  }
                  if (pieces[i][j][y] == y) {
                     int dy = -1 + 2*(y/2);
                     int yop = (y + 2) % 4;
                     if (y%2==0 and (0 <= j+dy) and (j+dy < m) and pieces[i][j+dy][yop] == yop)
                        continue;
                     if (y%2==1 and (0 <= i-dy) and (i-dy < n) and pieces[i-dy][j][yop] == yop)
                        continue;
                  }
 
                  if ((next() % pd) < pn) {
                     std::swap(pieces[i][j][x], pieces[i][j][y]);
                     //goto nextpiece;
                  }
               }
            }
            nextx:;
         }
         nextpiece:;
      }
   }

   for (int i=0; i<n; i++) {
      for (int j=0; j<m; j++) {
         auto pattern_piece = piece<lw, sep, sl, bw>(pieces[i][j], pcolors[i][j]);
         pattern._image.view(i*ps, j*ps, ps, ps).paint(pattern_piece);
      }
   }

   pattern.write("rand-sft.png");

   return 0;
}

int main() {

   //piece_main();
   //rand_main();
   //path_main();
   sft_main();
}

