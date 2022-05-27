#ifndef BITSET_HPP
#define BITSET_HPP

#include <bit>
#include <string>
#include <sstream>
#include <vector>

template<typename block = unsigned int>
class bitset {
   static constexpr std::size_t blocksize = sizeof(block) << 3;
   static constexpr block set_block = static_cast<block>(-1);
   static constexpr block reset_block = static_cast<block>(0);
   std::size_t aloc;
   std::size_t N;
   block last_mask;
   std::vector<block> data;

public:
   bitset(std::size_t N): N(N) {
      aloc = (N + blocksize - 1)/blocksize;
      last_mask = set_block >> (blocksize - (N % blocksize));
      data = std::vector<block>(aloc, reset_block);
   }

   bool operator==(const bitset<block>& rhs) const {
      if (N != rhs.N)
         return false;
      for (int i = 0; i < aloc; i++)
         if (data[i] != rhs.data[i])
            return false;
      return true;
   }
      
   bool operator[](std::size_t pos) const {
      return (data[pos/blocksize] >> (pos%blocksize)) & 1;
   }

   bool test(std::size_t pos) const {
      if (pos >= N)
         throw std::out_of_range("Bitset index out of range");
      return (*this)[pos];
   }

   bool all() const {
      for (int i = 0; i < aloc-1; i++)
         if (data[i] != set_block)
            return false;
      return data[aloc-1] == last_mask;
   }

   bool any() const {
      for (int i = 0; i < aloc; i++)
         if (data[i] != reset_block)
            return true;
      return false;
   }

   std::size_t count() const {
      std::size_t num = 0;
      for (int i = 0; i < aloc; i++)
         num += std::popcount(data[i]);
      return num;
   }

   std::size_t size() const {
      return N;
   }

   bitset<block>& operator&=(const bitset<block>& other) {
      for (int i = 0; i < std::min(aloc, other.aloc); i++)
         data[i] &= other.data[i];
      return *this;
   }

   bitset<block> operator&(const bitset<block>& other) {
      bitset<block> res = this->copy();
      res &= other;
      return res;
   }

   bitset<block>& operator|=(const bitset<block>& other) {
      for (int i = 0; i < std::min(aloc, other.aloc); i++)
         data[i] |= other.data[i];
      if (aloc <= other.aloc)
         data[aloc-1] &= last_mask;
      return *this;
   }

   bitset<block> operator|(const bitset<block>& other) {
      bitset<block> res = this->copy();
      res |= other;
      return res;
   }

   bitset<block>& operator^=(const bitset<block>& other) {
      for (int i = 0; i < std::min(aloc, other.aloc); i++)
         data[i] ^= other.data[i];
      if (aloc <= other.aloc)
         data[aloc-1] &= last_mask;
      return *this;
   }

   bitset<block> operator^(const bitset<block>& other) {
      bitset<block> res = this->copy();
      res ^= other;
      return res;
   }

   bitset<block>& flip() {
      for (int i = 0; i < aloc-1; i++)
         data[i] = ~data[i];
      data[aloc-1] = (~data[aloc-1]) & last_mask;
      return *this;
   }

   bitset<block> operator~() const {
      bitset<block> res = this->copy();
      res.flip();
      return res;
   }

   bitset<block>& operator<<=(std::size_t pos) {
      std::size_t step = pos / blocksize;
      if (step > 0) {
         for (int i = aloc-1; i >= step; i--)
            data[i] = data[i-step];
         for (int i = step-1; i >= 0; i--)
            data[i] = reset_block;
      }
      pos %= blocksize;
      block c = reset_block;
      block nc;
      for (int i = step; i < aloc; i++) {
         nc = data[i] >> (blocksize - pos);
         data[i] = (data[i] << pos) | c;
         c = nc;
      }
      data[aloc-1] &= last_mask;
      return *this;
   }

   bitset<block>& operator>>=(std::size_t pos) {
      std::size_t step = pos / blocksize;
      if (step > 0) {
         for (int i = 0; i < aloc-step; i++)
            data[i] = data[i+step];
         for (int i = aloc-step; i < aloc; i++)
            data[i] = reset_block;
      }
      pos %= blocksize;
      block c = reset_block;
      block nc;
      for (int i = aloc-1-step; i >= 0; i--) {
         nc = data[i] << (blocksize - pos);
         data[i] = c | (data[i] >> pos);
         c = nc;
      }
      return *this;
   }

   bitset<block> copy() const {
      bitset<block> res(N);
      for (int i = 0; i < aloc; i++)
         res.data[i] = data[i];
      return res;
   }

   bitset<block> operator<<(std::size_t pos) const {
      bitset<block> res = this->copy();
      return res <<= pos;
   }

   bitset<block> operator>>(std::size_t pos) const {
      bitset<block> res = this->copy();
      return res >>= pos;
   }

   bitset<block>& set() {
      for (int i = 0; i < aloc-1; i++)
         data[i] = set_block;
      data[aloc-1] = last_mask;
      return *this;
   }

   bitset<block>& reset() {
      for (int i = 0; i < aloc; i++)
         data[i] = reset_block;
      return *this;
   }

   bitset<block>& reset(std::size_t pos) {
      std::size_t step = pos / blocksize;
      block mask = ~(1 << (pos % blocksize));
      data[step] &= mask;
      return *this;
   }

   bitset<block>& set(std::size_t pos, bool value = true) {
      if (!value)
         return this->reset(pos);
      std::size_t step = pos / blocksize;
      block mask = 1 << (pos % blocksize);
      data[step] |= mask;
      return *this;
   }

   bitset<block>& flip(size_t pos) {
      std::size_t step = pos / blocksize;
      block mask = 1 << (pos % blocksize);
      data[step] ^= mask;
      return *this;
   }

   std::string to_string(char zero = '0', char one = '1') const {
      std::stringstream ss;
      block curr = data[aloc-1];
      block mask = 1 << ((N % blocksize) - 1);
      while (mask != 0) {
         if (curr & mask)
            ss << one;
         else
            ss << zero;
         mask >>= 1;
      }
      for (int i = aloc-2; i >= 0; i--) {
         curr = data[i];
         mask = 1 << (blocksize-1);
         while (mask != 0) {
            if (curr & mask)
               ss << one;
            else
               ss << zero;
            mask >>= 1;
         }
      }
      return ss.str();
   }
};

#endif
