#ifndef _MPIGRAV_VEC3_INCLUDED
#define _MPIGRAV_VEC3_INCLUDED


#include <string>


template<class T>
class Vec3 {
  public:
    T x;
    T y;
    T z;

  public:
    Vec3(void) : x(0), y(0), z(0) {}
    Vec3(T const x, T const y, T const z) : x(x), y(y), z(z) {}

    std::string Str(void)  {
      std::stringstream ss;
      ss << "(" << this->x << ",\t" << this->y << ",\t" << this->z << ")";
      return ss.str();
    }
};


//====[MATH OPERATORS]=======================================================//

// Addition
template<class T>
Vec3<T> operator+(Vec3<T> lhs, Vec3<T> const& rhs) {
  lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z;
  return lhs;
}

// Subtraction
template<class T>
Vec3<T> operator-(Vec3<T> lhs, Vec3<T> const& rhs) {
  lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z;
  return lhs;
}

// Multiplication - vector
template<class T>
Vec3<T> operator*(Vec3<T> lhs, Vec3<T> const& rhs) {
  lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z;
  return lhs;
}

// Multiplication - single
template<class T>
Vec3<T> operator*(Vec3<T> lhs, T const& rhs) {
  lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs;
  return lhs;
}

// Division - vector
template<class T>
Vec3<T> operator/(Vec3<T> lhs, Vec3<T> const& rhs) {
  lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z;
  return lhs;
}

// Division - single
template<class T>
Vec3<T> operator/(Vec3<T> lhs, T const& rhs) {
  lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs;
  return lhs;
}


#endif // _MPIGRAV_VEC3_INCLUDED
