
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

struct timer {
   
private:
   std::chrono::time_point<std::chrono::steady_clock> start;

public:
   inline timer() {
      start = std::chrono::steady_clock::now();
   }

   inline double get_time() {
      auto end = std::chrono::steady_clock::now();
      std::chrono::duration<double> dur = end - start;
      return dur.count();
   }

};

#endif

