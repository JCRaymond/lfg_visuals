
#ifndef VEC3_HPP
#define VEC3_HPP

#include <cmath>
#include <iostream>

struct vec3 {
   double x;
   double y;
   double z;

   vec3() {}
   vec3(double x, double y, double z) : x(x), y(y), z(z) {}

   vec3 operator+(vec3 other) {
      return {x + other.x, y + other.y, z + other.z};
   }

   void operator+=(vec3 other) {
      x += other.x;
      y += other.y;
      z += other.z;
   }

   vec3 operator-() {
      return {-x, -y, -z};
   }

   vec3 operator-(vec3 other) {
      return {x - other.x, y - other.y, z - other.z};
   }

   void operator-=(vec3 other) {
      x -= other.x;
      y -= other.y;
      z -= other.z;
   }

   double operator*(vec3 other) {
      return x*other.x + y*other.y + z*other.z;
   }

   vec3 operator&(vec3 other) {
      return {y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x};
   }

   double norm() {
      return sqrt(x*x + y*y + z*z);
   }

   double norm2() {
      return x*x + y*y + z*z;
   }

   vec3 normalized() {
      return (1/norm())*(*this);
   }

   friend vec3 operator*(double c, const vec3& v); 
   friend std::ostream& operator<<(std::ostream& out, const vec3& v);
};

vec3 operator*(double c, const vec3& v) {
   return {c*v.x, c*v.y, c*v.z};
}

std::ostream& operator<<(std::ostream& out, const vec3& v) {
   return out << '{' << v.x << ", " << v.y << ", " << v.z << '}';
}

#endif

