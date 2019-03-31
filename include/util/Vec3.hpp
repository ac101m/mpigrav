#ifndef _MPIGRAV_VEC3_INCLUDED
#define _MPIGRAV_VEC3_INCLUDED

#include <string>
#include <sstream>

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


// Math operators
Vec3 operator+(Vec3 lhs, Vec3 const& rhs);
Vec3 operator-(Vec3 lhs, Vec3 const& rhs);
Vec3 operator*(Vec3 lhs, Vec3 const& rhs);
Vec3 operator*(Vec3 lhs, fp_t const& rhs);
Vec3 operator/(Vec3 lhs, Vec3 const& rhs);
Vec3 operator/(Vec3 lhs, fp_t const& rhs);

// Comparison
bool operator!=(Vec3 const& lhs, Vec3 const& rhs);
bool operator==(Vec3 const& lhs, Vec3 const& rhs);

// General
Vec3 Normalize(Vec3 lhs);
fp_t Magnitude(Vec3 const& lhs);


#endif // _MPIGRAV_VEC3_INCLUDED
