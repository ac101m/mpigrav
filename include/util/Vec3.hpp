#ifndef _MPIGRAV_VEC3_INCLUDED
#define _MPIGRAV_VEC3_INCLUDED

#include <string>
#include <sstream>
#include <cmath>

#include "Master.hpp"


class Vec3 {
  public:
    float x;
    float y;
    float z;

  public:
    Vec3(void) : x(0), y(0), z(0) {}
    Vec3(float const x, float const y, float const z) : x(x), y(y), z(z) {}

    std::string Str(void)  {
      std::stringstream ss;
      ss << "(" << this->x << ",\t" << this->y << ",\t" << this->z << ")";
      return ss.str();
    }
};


//====[ARITHMETIC]===========================================================//

// Addition
inline Vec3 operator+(Vec3 lhs, Vec3 const& rhs) {
  lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z;
  return lhs;
}

// Subtraction
inline Vec3 operator-(Vec3 lhs, Vec3 const& rhs) {
  lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z;
  return lhs;
}

// Multiplication - vector
inline Vec3 operator*(Vec3 lhs, Vec3 const& rhs) {
  lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z;
  return lhs;
}

// Multiplication - single
inline Vec3 operator*(Vec3 lhs, float const& rhs) {
  lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs;
  return lhs;
}

// Division - vector
inline Vec3 operator/(Vec3 lhs, Vec3 const& rhs) {
  lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z;
  return lhs;
}

// Division - single
inline Vec3 operator/(Vec3 lhs, float const& rhs) {
  lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs;
  return lhs;
}


//====[COMPARISON]===========================================================//

// Not equal
inline bool operator!=(Vec3 const& lhs, Vec3 const& rhs) {
  return (lhs.x != rhs.x) || (lhs.y != rhs.y) || (lhs.z != rhs.z);
}

// Equal
inline bool operator==(Vec3 const& lhs, Vec3 const& rhs) {
  return !(lhs != rhs);
}

//====[GENERAL]==============================================================//

// Normalize
inline Vec3 Normalize(Vec3 lhs) {
  float length;
  length = sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);
  lhs.x /= length;
  lhs.y /= length;
  lhs.z /= length;
  return lhs;
}


// Get magnitude of vector
inline float Magnitude(Vec3 const& lhs) {
  return sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);
}


#endif // _MPIGRAV_VEC3_INCLUDED
