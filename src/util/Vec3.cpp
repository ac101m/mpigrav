#include "util/Vec3.hpp"

#include <cmath>


//====[ARITHMETIC]===========================================================//

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
Vec3 operator*(Vec3 lhs, float const& rhs) {
  lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs;
  return lhs;
}

// Division - vector
Vec3 operator/(Vec3 lhs, Vec3 const& rhs) {
  lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z;
  return lhs;
}

// Division - single
Vec3 operator/(Vec3 lhs, float const& rhs) {
  lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs;
  return lhs;
}


//====[COMPARISON]===========================================================//

// Not equal
bool operator!=(Vec3 const& lhs, Vec3 const& rhs) {
  return (lhs.x != rhs.x) || (lhs.y != rhs.y) || (lhs.z != rhs.z);
}

// Equal
bool operator==(Vec3 const& lhs, Vec3 const& rhs) {
  return !(lhs != rhs);
}

//====[GENERAL]==============================================================//

// Normalize
Vec3 Normalize(Vec3 lhs) {
  float length;
  length = sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);
  lhs.x /= length;
  lhs.y /= length;
  lhs.z /= length;
  return lhs;
}


// Get magnitude of vector
float Magnitude(Vec3 const& lhs) {
  return sqrt(lhs.x*lhs.x + lhs.y*lhs.y + lhs.z*lhs.z);
}
