#include "util/Vec3.hpp"


// Addition
Vec3 operator+(Vec3 lhs, Vec3 const& rhs) {
  lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z;
  return lhs;
}

// Subtraction
Vec3 operator-(Vec3 lhs, Vec3 const& rhs) {
  lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z;
  return lhs;
}

// Multiplication - vector
Vec3 operator*(Vec3 lhs, Vec3 const& rhs) {
  lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z;
  return lhs;
}

// Multiplication - single
Vec3 operator*(Vec3 lhs, fp_t const& rhs) {
  lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs;
  return lhs;
}

// Division - vector
Vec3 operator/(Vec3 lhs, Vec3 const& rhs) {
  lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z;
  return lhs;
}

// Division - single
Vec3 operator/(Vec3 lhs, fp_t const& rhs) {
  lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs;
  return lhs;
}
